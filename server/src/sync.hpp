#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

namespace ndtsync
{
    /// @brief Ошибка в запросе клиента - уходит ответом 400 с этим текстом, база не меняется
    struct BadRequest : std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /// @brief Принять пачку изменённых записей одной транзакцией.
    /// Каждая запись получает новый номер версии; правка поверх чужой версии принимается,
    /// а затёртая версия копируется в overwritten
    /// @param body {"records": [{tbl, id, base_rev, updated_at, deleted_at, data}, ...]}
    /// @return {"accepted": [{tbl, id, rev, conflict}, ...]}
    nlohmann::json Push(pqxx::connection &conn, const std::string &deviceId, const nlohmann::json &body);

    /// @brief Записи с номером версии больше since, по возрастанию, не больше limit
    /// @return {"records": [...], "last_rev": N, "more": bool}
    nlohmann::json Pull(pqxx::connection &conn, std::int64_t since, int limit);

    /// @brief Сохранить файл. Файл с таким id уже есть - ничего не делает: содержимое под одним id не меняется
    void PutFile(pqxx::connection &conn, const std::string &deviceId, const std::string &id,
                 const std::string &name, std::string_view data);

    struct StoredFile
    {
        std::string name;
        std::string data;
    };

    /// @brief Прочитать файл; nullopt - такого нет
    std::optional<StoredFile> GetFile(pqxx::connection &conn, const std::string &id);
}
