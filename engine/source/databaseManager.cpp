#include "databaseManager.hpp"

#include <SDL3/SDL.h>
#include "sqlite3.h"
#include "laboratory.hpp"

DatabaseManager::DatabaseManager(const std::filesystem::path &pathToDb)
{
    if (sqlite3_open_v2(pathToDb.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open database '%s': %s",
                     pathToDb.string().c_str(), sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return;
    }

    SDL_Log("Database '%s' opened successfully.", pathToDb.string().c_str());

    EnsureEmployeesTable();
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(db);
}

namespace
{
    const std::vector<std::pair<std::string, std::string>> employeeColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"name", "TEXT"},
        {"organization", "TEXT"},
        {"department", "TEXT"},
        {"position", "TEXT"},
        {"employeement_date", "TEXT"},
        {"personal_code", "TEXT"},
        {"level", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_date", "TEXT"},
        {"certificate_end_vt", "TEXT"},
        {"certificate_end_ut", "TEXT"},
        {"certificate_end_rt", "TEXT"},
        {"certificate_end_pt", "TEXT"},
        {"certificate_end_mt", "TEXT"},
        {"certificate_end_lt", "TEXT"},
    };

    /// @brief sqlite3_column_text возвращает nullptr для NULL-значения (например, у старых строк
    /// после ALTER TABLE ADD COLUMN) - присваивание nullptr в std::string это UB/сегфолт
    std::string GetColumnText(sqlite3_stmt *stmt, int col)
    {
        const unsigned char *text = sqlite3_column_text(stmt, col);
        return text ? reinterpret_cast<const char *>(text) : std::string{};
    }
}

void DatabaseManager::EnsureEmployeesTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS employees (";
    for (size_t i = 0; i < employeeColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += employeeColumns[i].first + " " + employeeColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureEmployeesTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    // Если таблица уже существовала (создана более старой версией приложения без каких-то полей) -
    // CREATE TABLE IF NOT EXISTS её не тронул, поэтому дозаполняем недостающие колонки поштучно.
    // "duplicate column name" здесь ожидаема и безопасно игнорируется - колонка уже есть.
    for (size_t i = 1; i < employeeColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE employees ADD COLUMN " + employeeColumns[i].first + " " + employeeColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::SaveEmployees(const std::vector<Employee> &employees)
{
    if (!db)
        return;

    EnsureEmployeesTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < employeeColumns.size(); ++i)
    {
        const std::string &name = employeeColumns[i].first;

        if (i > 0)
        {
            columnNames += ", ";
            placeholders += ", ";
        }
        columnNames += name;
        placeholders += "?";

        if (name != "id") // первичный ключ не обновляем при конфликте, только вставляем один раз
        {
            if (!updateSet.empty())
                updateSet += ", ";
            updateSet += name + " = excluded." + name;
        }
    }

    std::string insertSql = "INSERT INTO employees (" + columnNames + ") VALUES (" + placeholders + ") "
                                                                                                    "ON CONFLICT(id) DO UPDATE SET " +
                            updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveEmployees: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Employee &e : employees)
    {
        sqlite3_bind_text(stmt, 1, e.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, e.updatedAt.time_since_epoch().count());

        if (e.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, e.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, e.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, e.organization.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, e.department.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, e.position.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, e.employeementDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, e.personalCode.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, e.level.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, e.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, e.certificateDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, e.certificateEndDateVT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, e.certificateEndDateUT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 15, e.certificateEndDateRT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 16, e.certificateEndDatePT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 17, e.certificateEndDateMT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 18, e.certificateEndDateLT.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveEmployees: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Employees saved.");
}

std::vector<Employee> DatabaseManager::LoadEmployees()
{
    std::vector<Employee> employees;

    if (!db)
        return employees;

    std::string columnNames;
    for (size_t i = 0; i < employeeColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += employeeColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM employees;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadEmployees: prepare не удался: %s", sqlite3_errmsg(db));
        return employees;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Employee e;

        e.id = GetColumnText(stmt, 0);
        e.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            e.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        e.name = GetColumnText(stmt, 3);
        e.organization = GetColumnText(stmt, 4);
        e.department = GetColumnText(stmt, 5);
        e.position = GetColumnText(stmt, 6);
        e.employeementDate = GetColumnText(stmt, 7);
        e.personalCode = GetColumnText(stmt, 8);
        e.level = GetColumnText(stmt, 9);
        e.certificateNumber = GetColumnText(stmt, 10);
        e.certificateDate = GetColumnText(stmt, 11);
        e.certificateEndDateVT = GetColumnText(stmt, 12);
        e.certificateEndDateUT = GetColumnText(stmt, 13);
        e.certificateEndDateRT = GetColumnText(stmt, 14);
        e.certificateEndDatePT = GetColumnText(stmt, 15);
        e.certificateEndDateMT = GetColumnText(stmt, 16);
        e.certificateEndDateLT = GetColumnText(stmt, 17);

        employees.push_back(std::move(e));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Employees loaded.");
    return employees;
}
