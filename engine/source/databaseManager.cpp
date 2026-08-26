#include "databaseManager.hpp"

#include <SDL3/SDL.h>
#include "sqlite3.h"
#include "laboratory.hpp"

DatabaseManager::DatabaseManager(const std::filesystem::path &pathToDb)
{
    // sqlite3_open_v2 ожидает путь в UTF-8; path::string() на Windows конвертирует
    // в текущую ANSI-кодировку системы и портит некириллические/нелатинские символы пути
    const std::u8string pathUtf8 = pathToDb.u8string();

    if (sqlite3_open_v2(reinterpret_cast<const char *>(pathUtf8.c_str()), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open database '%s': %s",
                     pathToDb.string().c_str(), sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return;
    }

    SDL_Log("Database '%s' opened successfully.", pathToDb.string().c_str());

    EnsureLaboratoryInfoTable();
    EnsureEmployeesTable();
    EnsureInspectorsTable();
    EnsureMastersTable();
    EnsureWeldersTable();
    EnsureEquipmentTable();
    EnsureControlMapsTable();
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(db);
}

namespace
{
    const std::vector<std::pair<std::string, std::string>> laboratoryInfoColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"laboratory_name", "TEXT"},
        {"number_attestation", "TEXT"},
        {"attestation_end_date", "TEXT"},
    };

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

    const std::vector<std::pair<std::string, std::string>> masterColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"name", "TEXT"},
        {"organization", "TEXT"},
        {"department", "TEXT"},
        {"position", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_end_date", "TEXT"},
    };

    const std::vector<std::pair<std::string, std::string>> welderColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"name", "TEXT"},
        {"organization", "TEXT"},
        {"department", "TEXT"},
        {"position", "TEXT"},
        {"personal_code", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_end_date", "TEXT"},
    };

    const std::vector<std::pair<std::string, std::string>> controlMapColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"code", "TEXT"},
        {"method", "TEXT"},
        {"diameter", "TEXT"},
        {"thickness", "TEXT"},
        {"description", "TEXT"},
        {"file_name", "TEXT"},
        {"file_data", "BLOB"},
    };

    const std::vector<std::pair<std::string, std::string>> equipmentColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"name", "TEXT"},
        {"method", "TEXT"},
        {"function", "TEXT"},
        {"manufacturer", "TEXT"},
        {"serial_number", "TEXT"},
        {"year_of_manufacture", "TEXT"},
        {"year_of_commissioning", "TEXT"},
        {"technical_and_metrological_characteristics", "TEXT"},
        {"owner", "TEXT"},
        {"certificate_number", "TEXT"},
        {"certificate_date", "TEXT"},
        {"certificate_end_date", "TEXT"},
        {"state", "TEXT"},
        {"for_vt", "INTEGER DEFAULT 0"},
        {"for_ut", "INTEGER DEFAULT 0"},
        {"for_rt", "INTEGER DEFAULT 0"},
        {"for_drt", "INTEGER DEFAULT 0"},
        {"for_pt", "INTEGER DEFAULT 0"},
        {"for_mt", "INTEGER DEFAULT 0"},
        {"for_lt", "INTEGER DEFAULT 0"},
        {"for_ect", "INTEGER DEFAULT 0"},
        {"is_operational", "INTEGER DEFAULT 1"},
        {"is_under_repair", "INTEGER DEFAULT 0"},
        {"is_faulty", "INTEGER DEFAULT 0"},
        {"is_pending_disposal", "INTEGER DEFAULT 0"},
        {"is_preserved", "INTEGER DEFAULT 0"},
        {"is_calibrated", "INTEGER DEFAULT 0"},
        {"file_name", "TEXT"},
        {"file_data", "BLOB"},
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

    /// @brief Аналогично GetColumnText, но для BLOB-колонки
    std::vector<std::uint8_t> GetColumnBlob(sqlite3_stmt *stmt, int col)
    {
        const void *data = sqlite3_column_blob(stmt, col);
        int size = sqlite3_column_bytes(stmt, col);
        if (!data || size <= 0)
            return {};

        const std::uint8_t *bytes = static_cast<const std::uint8_t *>(data);
        return std::vector<std::uint8_t>(bytes, bytes + size);
    }
}

void DatabaseManager::EnsureLaboratoryInfoTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS laboratory_info (";
    for (size_t i = 0; i < laboratoryInfoColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += laboratoryInfoColumns[i].first + " " + laboratoryInfoColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureLaboratoryInfoTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < laboratoryInfoColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE laboratory_info ADD COLUMN " + laboratoryInfoColumns[i].first + " " + laboratoryInfoColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
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

void DatabaseManager::EnsureMastersTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS masters (";
    for (size_t i = 0; i < masterColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += masterColumns[i].first + " " + masterColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureMastersTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < masterColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE masters ADD COLUMN " + masterColumns[i].first + " " + masterColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::EnsureWeldersTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS welders (";
    for (size_t i = 0; i < welderColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += welderColumns[i].first + " " + welderColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureWeldersTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < welderColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE welders ADD COLUMN " + welderColumns[i].first + " " + welderColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::EnsureEquipmentTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS equipment (";
    for (size_t i = 0; i < equipmentColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += equipmentColumns[i].first + " " + equipmentColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureEquipmentTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < equipmentColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE equipment ADD COLUMN " + equipmentColumns[i].first + " " + equipmentColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::EnsureControlMapsTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS control_maps (";
    for (size_t i = 0; i < controlMapColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += controlMapColumns[i].first + " " + controlMapColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureControlMapsTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < controlMapColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE control_maps ADD COLUMN " + controlMapColumns[i].first + " " + controlMapColumns[i].second + ";";
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

void DatabaseManager::SaveMasters(const std::vector<Master> &masters)
{
    if (!db)
        return;

    EnsureMastersTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < masterColumns.size(); ++i)
    {
        const std::string &name = masterColumns[i].first;

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

    std::string insertSql = "INSERT INTO masters (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveMasters: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Master &m : masters)
    {
        sqlite3_bind_text(stmt, 1, m.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, m.updatedAt.time_since_epoch().count());

        if (m.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, m.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, m.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, m.organization.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, m.department.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, m.position.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, m.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, m.certificateEndDate.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveMasters: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Masters saved.");
}

std::vector<Master> DatabaseManager::LoadMasters()
{
    std::vector<Master> masters;

    if (!db)
        return masters;

    std::string columnNames;
    for (size_t i = 0; i < masterColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += masterColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM masters;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadMasters: prepare не удался: %s", sqlite3_errmsg(db));
        return masters;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Master m;

        m.id = GetColumnText(stmt, 0);
        m.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            m.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        m.name = GetColumnText(stmt, 3);
        m.organization = GetColumnText(stmt, 4);
        m.department = GetColumnText(stmt, 5);
        m.position = GetColumnText(stmt, 6);
        m.certificateNumber = GetColumnText(stmt, 7);
        m.certificateEndDate = GetColumnText(stmt, 8);

        masters.push_back(std::move(m));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Masters loaded.");
    return masters;
}

void DatabaseManager::SaveWelders(const std::vector<Welder> &welders)
{
    if (!db)
        return;

    EnsureWeldersTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < welderColumns.size(); ++i)
    {
        const std::string &name = welderColumns[i].first;

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

    std::string insertSql = "INSERT INTO welders (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveWelders: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Welder &w : welders)
    {
        sqlite3_bind_text(stmt, 1, w.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, w.updatedAt.time_since_epoch().count());

        if (w.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, w.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, w.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, w.organization.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, w.department.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, w.position.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, w.personalCode.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, w.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, w.certificateEndDate.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveWelders: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Welders saved.");
}

std::vector<Welder> DatabaseManager::LoadWelders()
{
    std::vector<Welder> welders;

    if (!db)
        return welders;

    std::string columnNames;
    for (size_t i = 0; i < welderColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += welderColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM welders;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadWelders: prepare не удался: %s", sqlite3_errmsg(db));
        return welders;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Welder w;

        w.id = GetColumnText(stmt, 0);
        w.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            w.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        w.name = GetColumnText(stmt, 3);
        w.organization = GetColumnText(stmt, 4);
        w.department = GetColumnText(stmt, 5);
        w.position = GetColumnText(stmt, 6);
        w.personalCode = GetColumnText(stmt, 7);
        w.certificateNumber = GetColumnText(stmt, 8);
        w.certificateEndDate = GetColumnText(stmt, 9);

        welders.push_back(std::move(w));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Welders loaded.");
    return welders;
}

void DatabaseManager::SaveEquipment(const std::vector<Equipment> &equipmentList)
{
    if (!db)
        return;

    EnsureEquipmentTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < equipmentColumns.size(); ++i)
    {
        const std::string &name = equipmentColumns[i].first;

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

    std::string insertSql = "INSERT INTO equipment (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveEquipment: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Equipment &eq : equipmentList)
    {
        sqlite3_bind_text(stmt, 1, eq.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, eq.updatedAt.time_since_epoch().count());

        if (eq.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, eq.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, eq.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, eq.method.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, eq.function.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, eq.manufacturer.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, eq.serialNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, eq.yearOfManufacture.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 10, eq.yearOfCommissioning.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 11, eq.technicalAndMetrologicalCharacteristics.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 12, eq.owner.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 13, eq.certificateNumber.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 14, eq.certificateDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 15, eq.certificateEndDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 16, eq.state.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 17, eq.forVT ? 1 : 0);
        sqlite3_bind_int(stmt, 18, eq.forUT ? 1 : 0);
        sqlite3_bind_int(stmt, 19, eq.forRT ? 1 : 0);
        sqlite3_bind_int(stmt, 20, eq.forDRT ? 1 : 0);
        sqlite3_bind_int(stmt, 21, eq.forPT ? 1 : 0);
        sqlite3_bind_int(stmt, 22, eq.forMT ? 1 : 0);
        sqlite3_bind_int(stmt, 23, eq.forLT ? 1 : 0);
        sqlite3_bind_int(stmt, 24, eq.forECT ? 1 : 0);
        sqlite3_bind_int(stmt, 25, eq.isOperational ? 1 : 0);
        sqlite3_bind_int(stmt, 26, eq.isUnderRepair ? 1 : 0);
        sqlite3_bind_int(stmt, 27, eq.isFaulty ? 1 : 0);
        sqlite3_bind_int(stmt, 28, eq.isPendingDisposal ? 1 : 0);
        sqlite3_bind_int(stmt, 29, eq.isPreserved ? 1 : 0);
        sqlite3_bind_int(stmt, 30, eq.isCalibrated ? 1 : 0);
        sqlite3_bind_text(stmt, 31, eq.fileName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 32, eq.fileData.data(), static_cast<int>(eq.fileData.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveEquipment: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Equipment saved.");
}

std::vector<Equipment> DatabaseManager::LoadEquipment()
{
    std::vector<Equipment> equipmentList;

    if (!db)
        return equipmentList;

    std::string columnNames;
    for (size_t i = 0; i < equipmentColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += equipmentColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM equipment;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadEquipment: prepare не удался: %s", sqlite3_errmsg(db));
        return equipmentList;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Equipment eq;

        eq.id = GetColumnText(stmt, 0);
        eq.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            eq.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        eq.name = GetColumnText(stmt, 3);
        eq.method = GetColumnText(stmt, 4);
        eq.function = GetColumnText(stmt, 5);
        eq.manufacturer = GetColumnText(stmt, 6);
        eq.serialNumber = GetColumnText(stmt, 7);
        eq.yearOfManufacture = GetColumnText(stmt, 8);
        eq.yearOfCommissioning = GetColumnText(stmt, 9);
        eq.technicalAndMetrologicalCharacteristics = GetColumnText(stmt, 10);
        eq.owner = GetColumnText(stmt, 11);
        eq.certificateNumber = GetColumnText(stmt, 12);
        eq.certificateDate = GetColumnText(stmt, 13);
        eq.certificateEndDate = GetColumnText(stmt, 14);
        eq.state = GetColumnText(stmt, 15);
        eq.forVT = sqlite3_column_int(stmt, 16) != 0;
        eq.forUT = sqlite3_column_int(stmt, 17) != 0;
        eq.forRT = sqlite3_column_int(stmt, 18) != 0;
        eq.forDRT = sqlite3_column_int(stmt, 19) != 0;
        eq.forPT = sqlite3_column_int(stmt, 20) != 0;
        eq.forMT = sqlite3_column_int(stmt, 21) != 0;
        eq.forLT = sqlite3_column_int(stmt, 22) != 0;
        eq.forECT = sqlite3_column_int(stmt, 23) != 0;
        eq.isOperational = sqlite3_column_int(stmt, 24) != 0;
        eq.isUnderRepair = sqlite3_column_int(stmt, 25) != 0;
        eq.isFaulty = sqlite3_column_int(stmt, 26) != 0;
        eq.isPendingDisposal = sqlite3_column_int(stmt, 27) != 0;
        eq.isPreserved = sqlite3_column_int(stmt, 28) != 0;
        eq.isCalibrated = sqlite3_column_int(stmt, 29) != 0;
        eq.fileName = GetColumnText(stmt, 30);
        eq.fileData = GetColumnBlob(stmt, 31);

        equipmentList.push_back(std::move(eq));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Equipment loaded.");
    return equipmentList;
}

void DatabaseManager::SaveControlMaps(const std::vector<ControlMap> &controlMaps)
{
    if (!db)
        return;

    EnsureControlMapsTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < controlMapColumns.size(); ++i)
    {
        const std::string &name = controlMapColumns[i].first;

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

    std::string insertSql = "INSERT INTO control_maps (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveControlMaps: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const ControlMap &cm : controlMaps)
    {
        sqlite3_bind_text(stmt, 1, cm.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, cm.updatedAt.time_since_epoch().count());

        if (cm.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, cm.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, cm.code.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, cm.method.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, cm.diameter.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, cm.thickness.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, cm.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, cm.fileName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 10, cm.fileData.data(), static_cast<int>(cm.fileData.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveControlMaps: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Control maps saved.");
}

std::vector<ControlMap> DatabaseManager::LoadControlMaps()
{
    std::vector<ControlMap> controlMaps;

    if (!db)
        return controlMaps;

    std::string columnNames;
    for (size_t i = 0; i < controlMapColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += controlMapColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM control_maps;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadControlMaps: prepare не удался: %s", sqlite3_errmsg(db));
        return controlMaps;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ControlMap cm;

        cm.id = GetColumnText(stmt, 0);
        cm.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            cm.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        cm.code = GetColumnText(stmt, 3);
        cm.method = GetColumnText(stmt, 4);
        cm.diameter = GetColumnText(stmt, 5);
        cm.thickness = GetColumnText(stmt, 6);
        cm.description = GetColumnText(stmt, 7);
        cm.fileName = GetColumnText(stmt, 8);
        cm.fileData = GetColumnBlob(stmt, 9);

        controlMaps.push_back(std::move(cm));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Control maps loaded.");
    return controlMaps;
}

void DatabaseManager::SaveLaboratoryInfo(const Laboratory &lab)
{
    if (!db)
        return;

    EnsureLaboratoryInfoTable();

    std::string insertSql = "INSERT INTO laboratory_info (id, updated_at, deleted_at, laboratory_name, number_attestation, attestation_end_date) "
                            "VALUES (?, ?, ?, ?, ?, ?) ON CONFLICT(id) DO UPDATE SET "
                            "updated_at = excluded.updated_at, deleted_at = excluded.deleted_at, "
                            "laboratory_name = excluded.laboratory_name, number_attestation = excluded.number_attestation, "
                            "attestation_end_date = excluded.attestation_end_date;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveLaboratoryInfo: prepare не удался: %s", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, lab.labInfo.id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, lab.labInfo.updatedAt.time_since_epoch().count());

    if (lab.labInfo.deletedAt.has_value())
        sqlite3_bind_int64(stmt, 3, lab.labInfo.deletedAt->time_since_epoch().count());
    else
        sqlite3_bind_null(stmt, 3);

    sqlite3_bind_text(stmt, 4, lab.labInfo.laboratoryName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, lab.labInfo.numberAttestation.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, lab.labInfo.attestationEndDate.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE)
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveLaboratoryInfo: вставка/обновление не удались: %s", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Laboratory info saved.");
}

void DatabaseManager::LoadLaboratoryInfo(Laboratory &lab)
{
    if (!db)
        return;

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT id, updated_at, deleted_at, laboratory_name, number_attestation, attestation_end_date FROM laboratory_info LIMIT 1;", -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadLaboratoryInfo: prepare не удался: %s", sqlite3_errmsg(db));
        return;
    }

    // если строки ещё нет (первый запуск) - оставляем lab.id таким, каким его сгенерировал
    // конструктор Laboratory; первый SaveDB() создаст именно эту запись
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        lab.labInfo.id = GetColumnText(stmt, 0);
        lab.labInfo.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            lab.labInfo.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        lab.labInfo.laboratoryName = GetColumnText(stmt, 3);
        lab.labInfo.numberAttestation = GetColumnText(stmt, 4);
        lab.labInfo.attestationEndDate = GetColumnText(stmt, 5);
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Laboratory info loaded.");
}
