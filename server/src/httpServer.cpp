#include "httpServer.hpp"

#include <cstdio>
#include <exception>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

#include "devices.hpp"

using json = nlohmann::json;

namespace
{
    void SendJson(httplib::Response &res, const json &body, int status = 200)
    {
        res.status = status;
        res.set_content(body.dump(), "application/json; charset=utf-8");
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

        /// исключение в обработчике (база недоступна, ошибка SQL) - 500 и строка в журнале,
        /// подробности клиенту не уходят
        server.set_exception_handler([](const httplib::Request &req, httplib::Response &res, std::exception_ptr ep)
                                     {
                                         try
                                         {
                                             std::rethrow_exception(ep);
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

        std::fprintf(stderr, "ndtsync: слушаю %s:%d\n", host.c_str(), port);
        return server.listen(host, port);
    }
}
