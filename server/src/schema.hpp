#pragma once

#include <pqxx/pqxx>

namespace ndtsync
{
    /// @brief Создать таблицы сервера, если их ещё нет. Вызывается при каждом запуске -
    /// все операторы с IF NOT EXISTS, повторный вызов ничего не меняет
    void EnsureSchema(pqxx::connection &conn);
}
