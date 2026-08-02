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
        {"experience", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_end_vt", "TEXT"},
        {"certificate_end_ut", "TEXT"},
        {"certificate_end_rt", "TEXT"},
        {"certificate_end_pt", "TEXT"},
        {"certificate_end_mt", "TEXT"},
        {"certificate_end_lt", "TEXT"},
    };
}

void DatabaseManager::SaveEmployees(const std::vector<Employee> &employees)
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
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveEmployees: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

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
        sqlite3_bind_text(stmt, 11, e.experience.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, e.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, e.certificateEndDateVT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, e.certificateEndDateUT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 15, e.certificateEndRT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 16, e.certificateEndPT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 17, e.certificateEndMT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 18, e.certificateEndLT.c_str(), -1, SQLITE_TRANSIENT);

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

        e.id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        e.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            e.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        e.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        e.organization = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        e.department = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        e.position = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6));
        e.employeementDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7));
        e.personalCode = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8));
        e.level = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9));
        e.experience = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10));
        e.certificateNumber = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
        e.certificateEndDateVT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 12));
        e.certificateEndDateUT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 13));
        e.certificateEndRT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 14));
        e.certificateEndPT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 15));
        e.certificateEndMT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 16));
        e.certificateEndLT = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 17));

        employees.push_back(std::move(e));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Employees loaded.");
    return employees;
}
