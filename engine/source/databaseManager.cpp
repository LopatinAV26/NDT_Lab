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
    EnsureInspectorsTable();
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
        {"has_vt", "INTEGER DEFAULT 1"}, // DEFAULT 1 - у существующих строк после ALTER TABLE ADD COLUMN считаем, что допуск есть (как было раньше, до этих флагов)
        {"certificate_end_vt", "TEXT"},
        {"has_ut", "INTEGER DEFAULT 1"},
        {"certificate_end_ut", "TEXT"},
        {"has_rt", "INTEGER DEFAULT 1"},
        {"certificate_end_rt", "TEXT"},
        {"has_pt", "INTEGER DEFAULT 1"},
        {"certificate_end_pt", "TEXT"},
        {"has_mt", "INTEGER DEFAULT 1"},
        {"certificate_end_mt", "TEXT"},
        {"has_lt", "INTEGER DEFAULT 1"},
        {"certificate_end_lt", "TEXT"},
    };

    const std::vector<std::pair<std::string, std::string>> inspectorColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"name", "TEXT"},
        {"organization", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_date", "TEXT"},
        {"has_vt", "INTEGER DEFAULT 1"},
        {"certificate_end_vt", "TEXT"},
        {"has_ut", "INTEGER DEFAULT 1"},
        {"certificate_end_ut", "TEXT"},
        {"has_rt", "INTEGER DEFAULT 1"},
        {"certificate_end_rt", "TEXT"},
        {"has_pt", "INTEGER DEFAULT 1"},
        {"certificate_end_pt", "TEXT"},
        {"has_mt", "INTEGER DEFAULT 1"},
        {"certificate_end_mt", "TEXT"},
        {"has_lt", "INTEGER DEFAULT 1"},
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

void DatabaseManager::EnsureInspectorsTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS inspectors (";
    for (size_t i = 0; i < inspectorColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += inspectorColumns[i].first + " " + inspectorColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureInspectorsTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < inspectorColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE inspectors ADD COLUMN " + inspectorColumns[i].first + " " + inspectorColumns[i].second + ";";
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

    std::string insertSql = "INSERT INTO employees (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

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
        sqlite3_bind_int(stmt, 13, e.hasVT ? 1 : 0);
        sqlite3_bind_text(stmt, 14, e.certificateEndDateVT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 15, e.hasUT ? 1 : 0);
        sqlite3_bind_text(stmt, 16, e.certificateEndDateUT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 17, e.hasRT ? 1 : 0);
        sqlite3_bind_text(stmt, 18, e.certificateEndDateRT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 19, e.hasPT ? 1 : 0);
        sqlite3_bind_text(stmt, 20, e.certificateEndDatePT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 21, e.hasMT ? 1 : 0);
        sqlite3_bind_text(stmt, 22, e.certificateEndDateMT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 23, e.hasLT ? 1 : 0);
        sqlite3_bind_text(stmt, 24, e.certificateEndDateLT.c_str(), -1, SQLITE_TRANSIENT);

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
        e.hasVT = sqlite3_column_int(stmt, 12) != 0;
        e.certificateEndDateVT = GetColumnText(stmt, 13);
        e.hasUT = sqlite3_column_int(stmt, 14) != 0;
        e.certificateEndDateUT = GetColumnText(stmt, 15);
        e.hasRT = sqlite3_column_int(stmt, 16) != 0;
        e.certificateEndDateRT = GetColumnText(stmt, 17);
        e.hasPT = sqlite3_column_int(stmt, 18) != 0;
        e.certificateEndDatePT = GetColumnText(stmt, 19);
        e.hasMT = sqlite3_column_int(stmt, 20) != 0;
        e.certificateEndDateMT = GetColumnText(stmt, 21);
        e.hasLT = sqlite3_column_int(stmt, 22) != 0;
        e.certificateEndDateLT = GetColumnText(stmt, 23);

        employees.push_back(std::move(e));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Employees loaded.");
    return employees;
}

void DatabaseManager::SaveInspectors(const std::vector<Inspector> &inspectors)
{
    if (!db)
        return;

    EnsureInspectorsTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < inspectorColumns.size(); ++i)
    {
        const std::string &name = inspectorColumns[i].first;

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

    std::string insertSql = "INSERT INTO inspectors (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveInspectors: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Inspector &i : inspectors)
    {
        sqlite3_bind_text(stmt, 1, i.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, i.updatedAt.time_since_epoch().count());

        if (i.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, i.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, i.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, i.organization.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, i.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, i.certificateDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, i.hasVT ? 1 : 0);
        sqlite3_bind_text(stmt, 9, i.certificateEndDateVT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 10, i.hasUT ? 1 : 0);
        sqlite3_bind_text(stmt, 11, i.certificateEndDateUT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 12, i.hasRT ? 1 : 0);
        sqlite3_bind_text(stmt, 13, i.certificateEndDateRT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 14, i.hasPT ? 1 : 0);
        sqlite3_bind_text(stmt, 15, i.certificateEndDatePT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 16, i.hasMT ? 1 : 0);
        sqlite3_bind_text(stmt, 17, i.certificateEndDateMT.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 18, i.hasLT ? 1 : 0);
        sqlite3_bind_text(stmt, 19, i.certificateEndDateLT.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveInspectors: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Inspectors saved.");
}

std::vector<Inspector> DatabaseManager::LoadInspectors()
{
    std::vector<Inspector> inspectors;

    if (!db)
        return inspectors;

    std::string columnNames;
    for (size_t i = 0; i < inspectorColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += inspectorColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM inspectors;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadInspectors: prepare не удался: %s", sqlite3_errmsg(db));
        return inspectors;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Inspector i;

        i.id = GetColumnText(stmt, 0);
        i.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            i.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        i.name = GetColumnText(stmt, 3);
        i.organization = GetColumnText(stmt, 4);
        i.certificateNumber = GetColumnText(stmt, 5);
        i.certificateDate = GetColumnText(stmt, 6);
        i.hasVT = sqlite3_column_int(stmt, 7) != 0;
        i.certificateEndDateVT = GetColumnText(stmt, 8);
        i.hasUT = sqlite3_column_int(stmt, 9) != 0;
        i.certificateEndDateUT = GetColumnText(stmt, 10);
        i.hasRT = sqlite3_column_int(stmt, 11) != 0;
        i.certificateEndDateRT = GetColumnText(stmt, 12);
        i.hasPT = sqlite3_column_int(stmt, 13) != 0;
        i.certificateEndDatePT = GetColumnText(stmt, 14);
        i.hasMT = sqlite3_column_int(stmt, 15) != 0;
        i.certificateEndDateMT = GetColumnText(stmt, 16);
        i.hasLT = sqlite3_column_int(stmt, 17) != 0;
        i.certificateEndDateLT = GetColumnText(stmt, 18);

        inspectors.push_back(std::move(i));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Inspectors loaded.");
    return inspectors;
}
