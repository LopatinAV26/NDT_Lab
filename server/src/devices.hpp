#pragma once

#include <optional>
#include <string>

#include <pqxx/pqxx>

namespace ndtsync
{
    struct Device
    {
        std::string id;
        std::string name;
    };

    /// @brief Зарегистрировать компьютер
    /// @return токен для программы - выдаётся один раз, в базе остаётся только его хэш
    std::string AddDevice(pqxx::connection &conn, const std::string &name);

    /// @brief Отозвать доступ (украли ноутбук, сотрудник уволился)
    /// @return false - компьютера с таким id нет или он уже отозван
    bool RevokeDevice(pqxx::connection &conn, const std::string &id);

    /// @brief Вывести список компьютеров в stdout
    void PrintDevices(pqxx::connection &conn);

    /// @brief Найти действующий компьютер по токену из заголовка Authorization и отметить время обращения
    std::optional<Device> Authenticate(pqxx::connection &conn, const std::string &token);
}
