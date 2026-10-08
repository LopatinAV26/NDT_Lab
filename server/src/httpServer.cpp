#include "httpServer.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <exception>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "devices.hpp"
#include "sync.hpp"

using json = nlohmann::json;

namespace
{
    constexpr size_t maxPushSize = 10 * 1024 * 1024; ///< пачка записей - текст, 10 МБ хватит на тысячи
    constexpr size_t maxFileSize = 50 * 1024 * 1024; ///< скан или PDF; больше - скорее ошибка, чем документ
    constexpr int defaultPullLimit = 500;
    constexpr int maxPullLimit = 1000;

    /// путь файла: id - UUID, другие символы в нём не встречаются
    constexpr const char *filePath = R"(/files/([0-9a-f-]{1,64}))";

    /// @brief Имя файла для заголовка: в заголовках HTTP только ASCII, кириллицу кодируем %XX
    std::string PercentEncode(const std::string &s)
    {
        constexpr char digits[] = "0123456789ABCDEF";
        std::string out;
        for (const unsigned char c : s)
        {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                out += static_cast<char>(c);
            else
            {
                out += '%';
                out += digits[c >> 4];
                out += digits[c & 0x0F];
            }
        }
        return out;
    }

    /// @brief Целое из параметра запроса; нет параметра - defaultValue
    std::int64_t GetQueryInt(const httplib::Request &req, const char *name, std::int64_t defaultValue)
    {
        if (!req.has_param(name))
            return defaultValue;
        try
        {
            return std::stoll(req.get_param_value(name));
        }
        catch (const std::exception &)
        {
            throw ndtsync::BadRequest(std::string(name) + ": нужно целое число");
        }
    }

    void SendJson(httplib::Response &res, const json &body, int status = 200)
    {
        res.status = status;
        /// replace: текст ошибки разбора может оборвать русскую букву посередине - без замены
        /// dump бросил бы исключение уже вне обработчика, и процесс упал бы целиком
        res.set_content(body.dump(-1, ' ', false, json::error_handler_t::replace), "application/json; charset=utf-8");
    }

    /// @brief Токен из заголовка "Authorization: Bearer <токен>"; пусто - заголовка нет
    std::string GetBearerToken(const httplib::Request &req)
    {
        constexpr std::string_view prefix = "Bearer ";
        const std::string header = req.get_header_value("Authorization");
        return header.starts_with(prefix) ? header.substr(prefix.size()) : std::string{};
    }

    /// @brief Проверить токен запроса. Не прошёл - ответ 401 уже записан в res
    std::optional<ndtsync::Device> RequireDevice(pqxx::connection &conn, const httplib::Request &req, httplib::Response &res)
    {
        std::optional<ndtsync::Device> device = ndtsync::Authenticate(conn, GetBearerToken(req));
        if (!device)
            SendJson(res, {{"error", "unauthorized"}}, 401);
        return device;
    }
}

namespace ndtsync
{
    bool RunServer(const std::string &host, int port)
    {
        httplib::Server server;

        /// соединение с базой - на каждый запрос: нагрузка - единицы компьютеров,
        /// а пул соединений между потоками httplib - лишняя сложность
        server.Get("/health", [](const httplib::Request &, httplib::Response &res)
                   {
                       pqxx::connection conn;
                       pqxx::read_transaction tx(conn);
                       tx.exec1("SELECT 1;");
                       SendJson(res, {{"ok", true}}); });

        /// проверка токена из программы: "кто я для сервера"
        server.Get("/whoami", [](const httplib::Request &req, httplib::Response &res)
                   {
                       pqxx::connection conn;
                       if (const auto device = RequireDevice(conn, req, res))
                           SendJson(res, {{"id", device->id}, {"name", device->name}}); });

        server.Post("/sync/push", [](const httplib::Request &req, httplib::Response &res)
                    {
                        if (req.body.size() > maxPushSize)
                            return SendJson(res, {{"error", "пачка больше 10 МБ - отправляйте частями"}}, 413);

                        pqxx::connection conn;
                        if (const auto device = RequireDevice(conn, req, res))
                            SendJson(res, ndtsync::Push(conn, device->id, json::parse(req.body))); });

        server.Get("/sync/pull", [](const httplib::Request &req, httplib::Response &res)
                   {
                       const std::int64_t since = GetQueryInt(req, "since", 0);
                       const int limit = static_cast<int>(std::clamp<std::int64_t>(GetQueryInt(req, "limit", defaultPullLimit), 1, maxPullLimit));

                       pqxx::connection conn;
                       if (RequireDevice(conn, req, res))
                           SendJson(res, ndtsync::Pull(conn, since, limit)); });

        /// клиент обязан слать Content-Type: application/octet-stream - тело с типом
        /// x-www-form-urlencoded httplib разбирает как форму и режет на 8 КБ (ответ 413)
        server.Put(filePath, [](const httplib::Request &req, httplib::Response &res)
                   {
                       if (req.body.size() > maxFileSize)
                           return SendJson(res, {{"error", "файл больше 50 МБ"}}, 413);

                       pqxx::connection conn;
                       if (const auto device = RequireDevice(conn, req, res))
                       {
                           /// имя - параметром запроса: httplib сам раскодирует %XX, кириллица доходит целой
                           ndtsync::PutFile(conn, device->id, req.matches[1].str(), req.get_param_value("name"), req.body);
                           SendJson(res, {{"ok", true}});
                       } });

        server.Get(filePath, [](const httplib::Request &req, httplib::Response &res)
                   {
                       pqxx::connection conn;
                       if (!RequireDevice(conn, req, res))
                           return;

                       const auto file = ndtsync::GetFile(conn, req.matches[1].str());
                       if (!file)
                           return SendJson(res, {{"error", "файла нет"}}, 404);

                       res.set_header("X-File-Name", PercentEncode(file->name));
                       res.set_content(file->data, "application/octet-stream"); });

        /// ошибка клиента (неправильный JSON, пропущенное поле) - 400 с объяснением;
        /// остальное (база недоступна, ошибка SQL) - 500 и строка в журнале, подробности клиенту не уходят
        server.set_exception_handler([](const httplib::Request &req, httplib::Response &res, std::exception_ptr ep)
                                     {
                                         try
                                         {
                                             std::rethrow_exception(ep);
                                         }
                                         catch (const ndtsync::BadRequest &e)
                                         {
                                             return SendJson(res, {{"error", e.what()}}, 400);
                                         }
                                         catch (const json::exception &e)
                                         {
                                             return SendJson(res, {{"error", std::string("неправильный JSON: ") + e.what()}}, 400);
                                         }
                                         catch (const std::exception &e)
                                         {
                                             std::fprintf(stderr, "%s %s: %s\n", req.method.c_str(), req.path.c_str(), e.what());
                                         }
                                         catch (...)
                                         {
                                             std::fprintf(stderr, "%s %s: неизвестная ошибка\n", req.method.c_str(), req.path.c_str());
                                         }
                                         SendJson(res, {{"error", "internal"}}, 500); });

        server.set_logger([](const httplib::Request &req, const httplib::Response &res)
                          { std::fprintf(stderr, "%s %s -> %d\n", req.method.c_str(), req.path.c_str(), res.status); });

        /// без этого httplib отверг бы большой файл раньше, чем до него дойдёт обработчик со своим ответом
        server.set_payload_max_length(maxFileSize + 1);

        std::fprintf(stderr, "ndtsync: слушаю %s:%d\n", host.c_str(), port);
        return server.listen(host, port);
    }
}
