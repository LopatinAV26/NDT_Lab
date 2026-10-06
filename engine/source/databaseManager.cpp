#include "databaseManager.hpp"

#include <algorithm>
#include <SDL3/SDL.h>
#include "sqlite3.h"
#include "laboratory.hpp"
#include "report.hpp"

DatabaseManager::DatabaseManager(const std::filesystem::path &pathToDb)
{
    const std::string pathUtf8 = NDT::PathToUtf8(pathToDb);
    
    if (sqlite3_open_v2(pathUtf8.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open database '%s': %s",
                     pathUtf8.c_str(), sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return;
    }

    SDL_Log("Database '%s' opened successfully.", pathUtf8.c_str());

    EnsureLaboratoryInfoTable();
    EnsureEmployeesTable();
    EnsureInspectorsTable();
    EnsureMastersTable();
    EnsureWeldersTable();
    EnsureEquipmentTable();
    EnsureControlMapsTable();
    EnsureNormativeDocumentsTable();
    EnsureFilmMeasurementsTable();
    EnsureDefectsRtTable();
    EnsureReportsTable();
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(db);
}

bool DatabaseManager::IsChanged(const NDT::DbRecord &record) const
{
    return record.updatedAt >= syncedAt;
}

template <typename T>
int DatabaseManager::CountChanged(const std::vector<T> &records) const
{
    return static_cast<int>(std::ranges::count_if(records, [this](const T &record)
                                                  { return IsChanged(record); }));
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
        {"diameter", "INTEGER"},
        {"thickness", "REAL"},
        {"description", "TEXT"},
        {"for_vt", "INTEGER"},
        {"for_ut", "INTEGER"},
        {"for_rt", "INTEGER"},
        {"for_drt", "INTEGER"},
        {"for_pt", "INTEGER"},
        {"for_mt", "INTEGER"},
        {"for_lt", "INTEGER"},
        {"for_dt", "INTEGER"},
        {"category_b", "INTEGER"},
        {"category_i", "INTEGER"},
        {"category_ii", "INTEGER"},
        {"category_iii", "INTEGER"},
        {"category_iv", "INTEGER"},
        {"file_name", "TEXT"},
        {"file_data", "BLOB"},
    };

    const std::vector<std::pair<std::string, std::string>> normativeDocumentColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"code", "TEXT"},
        {"name", "TEXT"},
        {"method", "TEXT"},
        {"status", "TEXT"},
        {"year", "INTEGER"},
        {"for_vt", "INTEGER"},
        {"for_ut", "INTEGER"},
        {"for_rt", "INTEGER"},
        {"for_drt", "INTEGER"},
        {"for_pt", "INTEGER"},
        {"for_mt", "INTEGER"},
        {"for_lt", "INTEGER"},
        {"for_ect", "INTEGER"},
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

    const std::vector<std::pair<std::string, std::string>> filmMeasurementColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"report_id", "TEXT"},
        {"coord", "INTEGER"},
        {"sensitivity", "REAL"},
        {"weld_density", "REAL"},
        {"haz_density", "REAL"},
        {"density_diff", "REAL"},
    };

    const std::vector<std::pair<std::string, std::string>> defectRtColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"report_id", "TEXT"},
        {"symbol", "TEXT"},
        {"length", "REAL"},
        {"width", "REAL"},
        {"height", "REAL"},
        {"end_greater_than", "INTEGER DEFAULT 0"},
        {"coord", "INTEGER"},
        {"exposure", "INTEGER DEFAULT 0"},
        {"acceptable", "INTEGER DEFAULT 1"},
    };

    const std::vector<std::pair<std::string, std::string>> reportColumns = {
        {"id", "TEXT PRIMARY KEY"},
        {"updated_at", "INTEGER"},
        {"deleted_at", "INTEGER"},
        {"method", "TEXT"},
        {"control_date", "TEXT"},
        {"report_date", "TEXT"},
        {"lab_name", "TEXT"},
        {"number_attestation", "TEXT"},
        {"weld_number", "TEXT"},
        {"report_number", "TEXT"},
        {"object_name", "TEXT"},
        {"pipe_category", "TEXT"},
        {"contractor_organization", "TEXT"},
        {"customer_organization", "TEXT"},
        {"controller_name", "TEXT"},
        {"controller_organization", "TEXT"},
        {"controller_cert_number", "TEXT"},
        {"protocol_create_name", "TEXT"},
        {"protocol_create_organization", "TEXT"},
        {"protocol_create_cert_number", "TEXT"},
        {"inspector_name", "TEXT"},
        {"inspector_organization", "TEXT"},
        {"inspector_cert_number", "TEXT"},
        {"master_name", "TEXT"},
        {"master_organization", "TEXT"},
        {"master_cert_number", "TEXT"},
        {"technological_control_map", "TEXT"},
        {"normative_docs", "TEXT"},
        {"normative_docs_ids", "TEXT"},
        {"equipment", "TEXT"},
        {"equipment_ids", "TEXT"},
        {"weld_type", "TEXT"},
        {"welding_methods", "TEXT"},
        {"diameter", "INTEGER"},
        {"perimeter", "INTEGER"},
        {"thickness1", "REAL"},
        {"thickness2", "REAL"},
        {"welders_mark", "TEXT"},
        {"welders_ids", "TEXT"},
        {"section_type1", "TEXT"},
        {"section_type2", "TEXT"},
        {"section_number1", "TEXT"},
        {"section_number2", "TEXT"},
        {"coord_sec1_weld1", "INTEGER"},
        {"coord_sec1_weld2", "INTEGER"},
        {"coord_sec2_weld1", "INTEGER"},
        {"coord_sec2_weld2", "INTEGER"},
        {"exposure_scheme", "TEXT"},
        {"ellipse_exposure_count", "INTEGER DEFAULT 2"},
        {"weld_opt_den_min", "REAL"},
        {"negatoscope_brightness", "INTEGER"},
        {"extent_of_unacceptable_defects", "REAL"},
        {"control_result", "TEXT"},
        {"section_notes", "TEXT"},
        {"brightness", "INTEGER"},
        {"temperature", "INTEGER"},
        {"roughness", "TEXT"},
        {"max_height_of_weld", "REAL"},
        {"min_height_of_weld", "REAL"},
        {"max_width_of_weld", "REAL"},
        {"min_width_of_weld", "REAL"},
        {"edge_displacement", "REAL"},
    };

    /// разделитель списка id в одной колонке: UUID запятых не содержит
    constexpr char idsSeparator = ',';

    /// разделитель примечаний по участкам: примечание вводится однострочным полем, перевода строки в нём не бывает
    constexpr char notesSeparator = '\n';

    /// @brief Склеить список строк в одну для колонки TEXT
    std::string JoinStrings(const std::vector<std::string> &values, char separator)
    {
        std::string result;
        for (size_t i = 0; i < values.size(); ++i)
        {
            if (i > 0)
                result += separator;
            result += values[i];
        }
        return result;
    }

    /// @brief Обратное к JoinStrings. Пустые элементы сохраняются - у примечаний важен номер участка,
    /// а пустая строка целиком даёт пустой список
    std::vector<std::string> SplitStrings(const std::string &value, char separator)
    {
        std::vector<std::string> result;
        if (value.empty())
            return result;

        size_t begin = 0;
        while (true)
        {
            const size_t end = value.find(separator, begin);
            result.push_back(value.substr(begin, end == std::string::npos ? std::string::npos : end - begin));
            if (end == std::string::npos)
                break;
            begin = end + 1;
        }
        return result;
    }

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

void DatabaseManager::EnsureNormativeDocumentsTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS normative_documents (";
    for (size_t i = 0; i < normativeDocumentColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += normativeDocumentColumns[i].first + " " + normativeDocumentColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureNormativeDocumentsTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < normativeDocumentColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE normative_documents ADD COLUMN " + normativeDocumentColumns[i].first + " " + normativeDocumentColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::EnsureFilmMeasurementsTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS film_measurements (";
    for (size_t i = 0; i < filmMeasurementColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += filmMeasurementColumns[i].first + " " + filmMeasurementColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureFilmMeasurementsTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < filmMeasurementColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE film_measurements ADD COLUMN " + filmMeasurementColumns[i].first + " " + filmMeasurementColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }

    // замеры всегда читаются пачкой по своему заключению - без индекса это полный перебор таблицы
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_film_measurements_report ON film_measurements(report_id);", nullptr, nullptr, nullptr);
}

void DatabaseManager::EnsureDefectsRtTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS defects_rt (";
    for (size_t i = 0; i < defectRtColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += defectRtColumns[i].first + " " + defectRtColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureDefectsRtTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < defectRtColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE defects_rt ADD COLUMN " + defectRtColumns[i].first + " " + defectRtColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }

    // дефекты читаются пачкой по своему заключению - как и замеры
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_defects_rt_report ON defects_rt(report_id);", nullptr, nullptr, nullptr);
}

void DatabaseManager::EnsureReportsTable()
{
    if (!db)
        return;

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS reports (";
    for (size_t i = 0; i < reportColumns.size(); ++i)
    {
        if (i > 0)
            createTableSql += ", ";

        createTableSql += reportColumns[i].first + " " + reportColumns[i].second;
    }
    createTableSql += ");";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createTableSql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "EnsureReportsTable: не удалось создать таблицу: %s", errMsg);
        sqlite3_free(errMsg);
        return;
    }

    for (size_t i = 1; i < reportColumns.size(); ++i) // с 1: id уже создан внутри CREATE TABLE выше
    {
        std::string alterSql = "ALTER TABLE reports ADD COLUMN " + reportColumns[i].first + " " + reportColumns[i].second + ";";
        sqlite3_exec(db, alterSql.c_str(), nullptr, nullptr, nullptr);
    }
}

void DatabaseManager::SaveEmployees(const std::vector<Employee> &employees)
{
    const int changedCount = CountChanged(employees);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(e)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Employees saved: %d", changedCount);
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
    const int changedCount = CountChanged(inspectors);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(i)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Inspectors saved: %d", changedCount);
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
    const int changedCount = CountChanged(masters);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(m)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Masters saved: %d", changedCount);
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
    const int changedCount = CountChanged(welders);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(w)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Welders saved: %d", changedCount);
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
    const int changedCount = CountChanged(equipmentList);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(eq)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

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

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Equipment saved: %d", changedCount);
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
    const int changedCount = CountChanged(controlMaps);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
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
        if (!IsChanged(cm)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

        sqlite3_bind_text(stmt, 1, cm.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, cm.updatedAt.time_since_epoch().count());

        if (cm.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, cm.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, cm.code.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, cm.diameter);
        sqlite3_bind_double(stmt, 6, cm.nominalWallThickness);
        sqlite3_bind_text(stmt, 7, cm.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, cm.forVT ? 1 : 0);
        sqlite3_bind_int(stmt, 9, cm.forUT ? 1 : 0);
        sqlite3_bind_int(stmt, 10, cm.forRT ? 1 : 0);
        sqlite3_bind_int(stmt, 11, cm.forDRT ? 1 : 0);
        sqlite3_bind_int(stmt, 12, cm.forPT ? 1 : 0);
        sqlite3_bind_int(stmt, 13, cm.forMT ? 1 : 0);
        sqlite3_bind_int(stmt, 14, cm.forLT ? 1 : 0);
        sqlite3_bind_int(stmt, 15, cm.forDT ? 1 : 0);
        sqlite3_bind_int(stmt, 16, cm.categoryB ? 1 : 0);
        sqlite3_bind_int(stmt, 17, cm.categoryI ? 1 : 0);
        sqlite3_bind_int(stmt, 18, cm.categoryII ? 1 : 0);
        sqlite3_bind_int(stmt, 19, cm.categoryIII ? 1 : 0);
        sqlite3_bind_int(stmt, 20, cm.categoryIV ? 1 : 0);
        sqlite3_bind_text(stmt, 21, cm.fileName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 22, cm.fileData.data(), static_cast<int>(cm.fileData.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveControlMaps: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Control maps saved: %d", changedCount);
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
        cm.diameter = sqlite3_column_int(stmt, 4);
        cm.nominalWallThickness = static_cast<float>(sqlite3_column_double(stmt, 5));
        cm.description = GetColumnText(stmt, 6);
        cm.forVT = sqlite3_column_int(stmt, 7) != 0;
        cm.forUT = sqlite3_column_int(stmt, 8) != 0;
        cm.forRT = sqlite3_column_int(stmt, 9) != 0;
        cm.forDRT = sqlite3_column_int(stmt, 10) != 0;
        cm.forPT = sqlite3_column_int(stmt, 11) != 0;
        cm.forMT = sqlite3_column_int(stmt, 12) != 0;
        cm.forLT = sqlite3_column_int(stmt, 13) != 0;
        cm.forDT = sqlite3_column_int(stmt, 14) != 0;
        cm.categoryB = sqlite3_column_int(stmt, 15) != 0;
        cm.categoryI = sqlite3_column_int(stmt, 16) != 0;
        cm.categoryII = sqlite3_column_int(stmt, 17) != 0;
        cm.categoryIII = sqlite3_column_int(stmt, 18) != 0;
        cm.categoryIV = sqlite3_column_int(stmt, 19) != 0;
        cm.fileName = GetColumnText(stmt, 20);
        cm.fileData = GetColumnBlob(stmt, 21);

        controlMaps.push_back(std::move(cm));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Control maps loaded.");
    return controlMaps;
}

void DatabaseManager::SaveNormativeDocuments(const std::vector<NormativeDocument> &normativeDocuments)
{
    const int changedCount = CountChanged(normativeDocuments);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
        return;

    EnsureNormativeDocumentsTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < normativeDocumentColumns.size(); ++i)
    {
        const std::string &name = normativeDocumentColumns[i].first;

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

    std::string insertSql = "INSERT INTO normative_documents (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveNormativeDocuments: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const NormativeDocument &doc : normativeDocuments)
    {
        if (!IsChanged(doc)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

        sqlite3_bind_text(stmt, 1, doc.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, doc.updatedAt.time_since_epoch().count());

        if (doc.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, doc.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        sqlite3_bind_text(stmt, 4, doc.code.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, doc.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, doc.method.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, doc.status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 8, doc.year);
        sqlite3_bind_int(stmt, 9, doc.forVT ? 1 : 0);
        sqlite3_bind_int(stmt, 10, doc.forUT ? 1 : 0);
        sqlite3_bind_int(stmt, 11, doc.forRT ? 1 : 0);
        sqlite3_bind_int(stmt, 12, doc.forDRT ? 1 : 0);
        sqlite3_bind_int(stmt, 13, doc.forPT ? 1 : 0);
        sqlite3_bind_int(stmt, 14, doc.forMT ? 1 : 0);
        sqlite3_bind_int(stmt, 15, doc.forLT ? 1 : 0);
        sqlite3_bind_int(stmt, 16, doc.forECT ? 1 : 0);
        sqlite3_bind_text(stmt, 17, doc.fileName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(stmt, 18, doc.fileData.data(), static_cast<int>(doc.fileData.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveNormativeDocuments: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Normative documents saved: %d", changedCount);
}

std::vector<NormativeDocument> DatabaseManager::LoadNormativeDocuments()
{
    std::vector<NormativeDocument> normativeDocuments;

    if (!db)
        return normativeDocuments;

    std::string columnNames;
    for (size_t i = 0; i < normativeDocumentColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += normativeDocumentColumns[i].first;
    }

    std::string selectSql = "SELECT " + columnNames + " FROM normative_documents;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadNormativeDocuments: prepare не удался: %s", sqlite3_errmsg(db));
        return normativeDocuments;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        NormativeDocument doc;

        doc.id = GetColumnText(stmt, 0);
        doc.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            doc.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        doc.code = GetColumnText(stmt, 3);
        doc.name = GetColumnText(stmt, 4);
        doc.method = GetColumnText(stmt, 5);
        doc.status = GetColumnText(stmt, 6);
        doc.year = sqlite3_column_int(stmt, 7);
        doc.forVT = sqlite3_column_int(stmt, 8) != 0;
        doc.forUT = sqlite3_column_int(stmt, 9) != 0;
        doc.forRT = sqlite3_column_int(stmt, 10) != 0;
        doc.forDRT = sqlite3_column_int(stmt, 11) != 0;
        doc.forPT = sqlite3_column_int(stmt, 12) != 0;
        doc.forMT = sqlite3_column_int(stmt, 13) != 0;
        doc.forLT = sqlite3_column_int(stmt, 14) != 0;
        doc.forECT = sqlite3_column_int(stmt, 15) != 0;
        doc.fileName = GetColumnText(stmt, 16);
        doc.fileData = GetColumnBlob(stmt, 17);

        normativeDocuments.push_back(std::move(doc));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Normative documents loaded.");
    return normativeDocuments;
}

void DatabaseManager::SaveFilmMeasurements(const std::string &reportId, const std::vector<FilmMeasurement> &measurements)
{
    if (!db || reportId.empty())
        return;

    EnsureFilmMeasurementsTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < filmMeasurementColumns.size(); ++i)
    {
        const std::string &name = filmMeasurementColumns[i].first;

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

    std::string insertSql = "INSERT INTO film_measurements (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveFilmMeasurements: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const FilmMeasurement &m : measurements)
    {
        sqlite3_bind_text(stmt, 1, m.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, m.updatedAt.time_since_epoch().count());

        if (m.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, m.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        /// связь берём из аргумента, а не из поля записи: замер приходит из вектора заключения
        /// и своего reportId может ещё не знать
        sqlite3_bind_text(stmt, 4, reportId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, m.coord);
        sqlite3_bind_double(stmt, 6, m.sensitivity);
        sqlite3_bind_double(stmt, 7, m.weldDensity);
        sqlite3_bind_double(stmt, 8, m.hazDensity);
        sqlite3_bind_double(stmt, 9, m.densityDiff);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveFilmMeasurements: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    // строки, которых в списке больше нет (таблица сократилась при смене схемы просвечивания),
    // удаляем физически - иначе следующая загрузка вернёт лишние замеры
    std::string deleteSql = "DELETE FROM film_measurements WHERE report_id = ?";
    if (!measurements.empty())
    {
        deleteSql += " AND id NOT IN (";
        for (size_t i = 0; i < measurements.size(); ++i)
            deleteSql += (i > 0) ? ", ?" : "?";
        deleteSql += ")";
    }
    deleteSql += ";";

    sqlite3_stmt *deleteStmt = nullptr;
    if (sqlite3_prepare_v2(db, deleteSql.c_str(), -1, &deleteStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(deleteStmt, 1, reportId.c_str(), -1, SQLITE_TRANSIENT);
        for (size_t i = 0; i < measurements.size(); ++i)
            sqlite3_bind_text(deleteStmt, static_cast<int>(i) + 2, measurements[i].id.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(deleteStmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveFilmMeasurements: удаление лишних замеров не удалось: %s", sqlite3_errmsg(db));
    }
    else
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveFilmMeasurements: prepare удаления не удался: %s", sqlite3_errmsg(db));

    sqlite3_finalize(deleteStmt);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Film measurements saved.");
}

std::vector<FilmMeasurement> DatabaseManager::LoadFilmMeasurements(const std::string &reportId)
{
    std::vector<FilmMeasurement> measurements;

    if (!db || reportId.empty())
        return measurements;

    std::string columnNames;
    for (size_t i = 0; i < filmMeasurementColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += filmMeasurementColumns[i].first;
    }

    // ORDER BY id: UUID v7 начинается с отметки времени, поэтому сортировка по нему
    // возвращает замеры в том порядке, в котором их создавали в форме
    std::string selectSql = "SELECT " + columnNames + " FROM film_measurements WHERE report_id = ? ORDER BY id;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadFilmMeasurements: prepare не удался: %s", sqlite3_errmsg(db));
        return measurements;
    }

    sqlite3_bind_text(stmt, 1, reportId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        FilmMeasurement m;

        m.id = GetColumnText(stmt, 0);
        m.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            m.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        m.reportId = GetColumnText(stmt, 3);
        m.coord = sqlite3_column_int(stmt, 4);
        m.sensitivity = static_cast<float>(sqlite3_column_double(stmt, 5));
        m.weldDensity = static_cast<float>(sqlite3_column_double(stmt, 6));
        m.hazDensity = static_cast<float>(sqlite3_column_double(stmt, 7));
        m.densityDiff = static_cast<float>(sqlite3_column_double(stmt, 8));

        measurements.push_back(std::move(m));
    }

    sqlite3_finalize(stmt);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Film measurements loaded.");
    return measurements;
}

void DatabaseManager::SaveDefectsRt(const std::string &reportId, const std::vector<DefectRt> &defects)
{
    if (!db || reportId.empty())
        return;

    EnsureDefectsRtTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < defectRtColumns.size(); ++i)
    {
        const std::string &name = defectRtColumns[i].first;

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

    std::string insertSql = "INSERT INTO defects_rt (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveDefectsRt: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const DefectRt &d : defects)
    {
        sqlite3_bind_text(stmt, 1, d.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, d.updatedAt.time_since_epoch().count());

        if (d.deletedAt.has_value())
            sqlite3_bind_int64(stmt, 3, d.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, 3);

        /// связь берём из аргумента, а не из поля записи - как у замеров
        sqlite3_bind_text(stmt, 4, reportId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, Report::GetDefectRTName(d.symbol).c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 6, d.length);
        sqlite3_bind_double(stmt, 7, d.width);
        sqlite3_bind_double(stmt, 8, d.height);
        sqlite3_bind_int(stmt, 9, d.endGreaterThan ? 1 : 0);
        sqlite3_bind_int(stmt, 10, d.coord);
        sqlite3_bind_int(stmt, 11, d.exposure);
        sqlite3_bind_int(stmt, 12, d.acceptable ? 1 : 0);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveDefectsRt: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    // дефекты, удалённые кнопкой в окне, стираем физически - иначе следующая загрузка вернёт их обратно
    std::string deleteSql = "DELETE FROM defects_rt WHERE report_id = ?";
    if (!defects.empty())
    {
        deleteSql += " AND id NOT IN (";
        for (size_t i = 0; i < defects.size(); ++i)
            deleteSql += (i > 0) ? ", ?" : "?";
        deleteSql += ")";
    }
    deleteSql += ";";

    sqlite3_stmt *deleteStmt = nullptr;
    if (sqlite3_prepare_v2(db, deleteSql.c_str(), -1, &deleteStmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(deleteStmt, 1, reportId.c_str(), -1, SQLITE_TRANSIENT);
        for (size_t i = 0; i < defects.size(); ++i)
            sqlite3_bind_text(deleteStmt, static_cast<int>(i) + 2, defects[i].id.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(deleteStmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveDefectsRt: удаление лишних дефектов не удалось: %s", sqlite3_errmsg(db));
    }
    else
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveDefectsRt: prepare удаления не удался: %s", sqlite3_errmsg(db));

    sqlite3_finalize(deleteStmt);

    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
}

std::vector<DefectRt> DatabaseManager::LoadDefectsRt(const std::string &reportId)
{
    std::vector<DefectRt> defects;

    if (!db || reportId.empty())
        return defects;

    std::string columnNames;
    for (size_t i = 0; i < defectRtColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += defectRtColumns[i].first;
    }

    // ORDER BY id: UUID v7 начинается с отметки времени - дефекты возвращаются в порядке ввода
    std::string selectSql = "SELECT " + columnNames + " FROM defects_rt WHERE report_id = ? ORDER BY id;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadDefectsRt: prepare не удался: %s", sqlite3_errmsg(db));
        return defects;
    }

    sqlite3_bind_text(stmt, 1, reportId.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        DefectRt d;

        d.id = GetColumnText(stmt, 0);
        d.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 1)}};

        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            d.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, 2)}};

        d.reportId = GetColumnText(stmt, 3);
        d.symbol = Report::ParseDefectRtSymbol(GetColumnText(stmt, 4));
        d.length = static_cast<float>(sqlite3_column_double(stmt, 5));
        d.width = static_cast<float>(sqlite3_column_double(stmt, 6));
        d.height = static_cast<float>(sqlite3_column_double(stmt, 7));
        d.endGreaterThan = sqlite3_column_int(stmt, 8) != 0;
        d.coord = sqlite3_column_int(stmt, 9);
        d.exposure = sqlite3_column_int(stmt, 10);
        d.acceptable = sqlite3_column_int(stmt, 11) != 0;

        defects.push_back(std::move(d));
    }

    sqlite3_finalize(stmt);
    return defects;
}

void DatabaseManager::SaveReports(const std::vector<Report> &reports)
{
    const int changedCount = CountChanged(reports);
    if (!db || changedCount == 0) /// нечего записывать - не открываем транзакцию впустую
        return;

    EnsureReportsTable();

    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

    std::string columnNames, placeholders, updateSet;

    for (size_t i = 0; i < reportColumns.size(); ++i)
    {
        const std::string &name = reportColumns[i].first;

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

    std::string insertSql = "INSERT INTO reports (" + columnNames + ") VALUES (" + placeholders + ") ON CONFLICT(id) DO UPDATE SET " + updateSet + ";";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveReports: prepare не удался: %s", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        return;
    }

    for (const Report &r : reports)
    {
        if (!IsChanged(r)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

        /// колонок за шестьдесят - номер параметра считаем счётчиком, а не пишем руками:
        /// вставка колонки в середину reportColumns иначе сдвинула бы все номера ниже
        int col = 1;
        const auto bindText = [&](const std::string &value)
        { sqlite3_bind_text(stmt, col++, value.c_str(), -1, SQLITE_TRANSIENT); };
        const auto bindInt = [&](int value)
        { sqlite3_bind_int(stmt, col++, value); };
        const auto bindDouble = [&](double value)
        { sqlite3_bind_double(stmt, col++, value); };

        bindText(r.id);
        sqlite3_bind_int64(stmt, col++, r.updatedAt.time_since_epoch().count());

        if (r.deletedAt.has_value())
            sqlite3_bind_int64(stmt, col++, r.deletedAt->time_since_epoch().count());
        else
            sqlite3_bind_null(stmt, col++);

        bindText(GetMethodAbbreviation(r.methodValue));
        bindText(r.controlDate);
        bindText(r.reportDate);
        bindText(r.nameLab);
        bindText(r.numberAttestation);
        bindText(r.weldNumber);
        bindText(r.reportNumber);
        bindText(r.objectName);
        bindText(GetCategoryStr(r.pipeCategory));
        bindText(r.contractorOrganization);
        bindText(r.customerOrganization);
        bindText(r.controllerName);
        bindText(r.controllerOrganization);
        bindText(r.controllerCertNumber);
        bindText(r.protocolCreateName);
        bindText(r.protocolCreateOrganization);
        bindText(r.protocolCreateCertNumber);
        bindText(r.inspectorName);
        bindText(r.inspectorOrganization);
        bindText(r.inspectorCertNumber);
        bindText(r.masterName);
        bindText(r.masterOrganization);
        bindText(r.masterCertNumber);
        bindText(r.technologicalControlMap);
        bindText(r.normativeDocs);
        bindText(JoinStrings(r.normativeDocsIds, idsSeparator));
        bindText(r.equipment);
        bindText(JoinStrings(r.equipmentIds, idsSeparator));
        bindText(GetWeldJointTypeStr(r.weldType));
        bindText(GetWeldingMethodsStr(r.weldingMethods));
        bindInt(r.diameter);
        bindInt(r.perimeter);
        bindDouble(r.thicknes1);
        bindDouble(r.thicknes2);
        bindText(r.weldersMark);
        bindText(JoinStrings(r.weldersIds, idsSeparator));
        bindText(GetSectionTypeStr(r.sectionType1));
        bindText(GetSectionTypeStr(r.sectionType2));
        bindText(r.sectionNumber1);
        bindText(r.sectionNumber2);
        bindInt(r.coordSec1Weld1);
        bindInt(r.coordSec1Weld2);
        bindInt(r.coordSec2Weld1);
        bindInt(r.coordSec2Weld2);
        bindText(GetExposureSchemeStr(r.exposureScheme));
        bindInt(r.ellipseExposureCount);
        bindDouble(r.weldOptDenMin);
        bindInt(r.negatoscopeBrightness);
        bindDouble(r.extentOfUnacceptableDefects);
        bindText(GetControlResultStr(r.controlResult));
        bindText(JoinStrings(r.sectionNotes, notesSeparator));
        bindInt(r.brightness);
        bindInt(r.temperature);
        bindText(GetRoughnessStr(r.roughness));
        bindDouble(r.maxHeightOfWeld);
        bindDouble(r.minHeightOfWeld);
        bindDouble(r.maxWidthOfWeld);
        bindDouble(r.minWidthOfWeld);
        bindDouble(r.edgeDisplacement);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SaveReports: вставка/обновление не удались: %s", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

    // дочерние таблицы - после COMMIT: у каждой своя транзакция, а вложенные транзакции SQLite не допускает
    for (const Report &r : reports)
    {
        if (!IsChanged(r)) /// не менялась с прошлого сохранения - в базе уже то же самое
            continue;

        SaveFilmMeasurements(r.id, r.filmMeasurements);
        SaveDefectsRt(r.id, r.defRGCList);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Reports saved: %d", changedCount);
}

std::vector<Report> DatabaseManager::LoadReports()
{
    std::vector<Report> reports;

    if (!db)
        return reports;

    std::string columnNames;
    for (size_t i = 0; i < reportColumns.size(); ++i)
    {
        if (i > 0)
            columnNames += ", ";
        columnNames += reportColumns[i].first;
    }

    // ORDER BY id: UUID v7 - заключения в списке идут в порядке создания
    std::string selectSql = "SELECT " + columnNames + " FROM reports ORDER BY id;";

    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "LoadReports: prepare не удался: %s", sqlite3_errmsg(db));
        return reports;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Report r;

        int col = 0; /// в том же порядке, что и reportColumns - см. SaveReports
        const auto text = [&]
        { return GetColumnText(stmt, col++); };
        const auto integer = [&]
        { return sqlite3_column_int(stmt, col++); };
        const auto real = [&]
        { return static_cast<float>(sqlite3_column_double(stmt, col++)); };

        r.id = text();
        r.updatedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, col++)}};

        if (sqlite3_column_type(stmt, col) != SQLITE_NULL)
            r.deletedAt = std::chrono::sys_seconds{std::chrono::seconds{sqlite3_column_int64(stmt, col)}};
        ++col;

        r.methodValue = ParseMethod(text());
        r.methodHeader = r.GetMethodReportTitle(r.methodValue); /// заголовок не хранится - он целиком следует из метода
        r.controlDate = text();
        r.reportDate = text();
        r.nameLab = text();
        r.numberAttestation = text();
        r.weldNumber = text();
        r.reportNumber = text();
        r.objectName = text();
        r.pipeCategory = ParseCategory(text());
        r.contractorOrganization = text();
        r.customerOrganization = text();
        r.controllerName = text();
        r.controllerOrganization = text();
        r.controllerCertNumber = text();
        r.protocolCreateName = text();
        r.protocolCreateOrganization = text();
        r.protocolCreateCertNumber = text();
        r.inspectorName = text();
        r.inspectorOrganization = text();
        r.inspectorCertNumber = text();
        r.masterName = text();
        r.masterOrganization = text();
        r.masterCertNumber = text();
        r.technologicalControlMap = text();
        r.normativeDocs = text();
        r.normativeDocsIds = SplitStrings(text(), idsSeparator);
        r.equipment = text();
        r.equipmentIds = SplitStrings(text(), idsSeparator);
        r.weldType = ParseWeldJointType(text());
        r.weldingMethods = ParseWeldingMethods(text());
        r.diameter = integer();
        r.perimeter = integer();
        r.thicknes1 = real();
        r.thicknes2 = real();
        r.weldersMark = text();
        r.weldersIds = SplitStrings(text(), idsSeparator);
        r.sectionType1 = ParseSectionType(text());
        r.sectionType2 = ParseSectionType(text());
        r.sectionNumber1 = text();
        r.sectionNumber2 = text();
        r.coordSec1Weld1 = integer();
        r.coordSec1Weld2 = integer();
        r.coordSec2Weld1 = integer();
        r.coordSec2Weld2 = integer();
        r.exposureScheme = ParseExposureScheme(text());
        r.ellipseExposureCount = std::clamp(integer(), GetFilmMeasurementCount(ExposureScheme::Ellipse), Report::maxEllipseExposures);
        r.weldOptDenMin = real();
        r.negatoscopeBrightness = integer();
        r.metalOptDenMax = NDT::GetMetalDensity(r.negatoscopeBrightness); /// предел не хранится - он следует из яркости
        r.extentOfUnacceptableDefects = real();
        r.controlResult = ParseControlResult(text());
        r.sectionNotes = SplitStrings(text(), notesSeparator);
        r.brightness = integer();
        r.temperature = integer();
        r.roughness = ParseRoughness(text());
        r.maxHeightOfWeld = real();
        r.minHeightOfWeld = real();
        r.maxWidthOfWeld = real();
        r.minWidthOfWeld = real();
        r.edgeDisplacement = real();

        reports.push_back(std::move(r));
    }

    sqlite3_finalize(stmt);

    for (Report &r : reports)
    {
        r.filmMeasurements = LoadFilmMeasurements(r.id);
        r.defRGCList = LoadDefectsRt(r.id);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", "Reports loaded.");
    return reports;
}

void DatabaseManager::SaveLaboratoryInfo(const Laboratory &lab)
{
    if (!db || !IsChanged(lab.labInfo))
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
    // конструктор Laboratory; первый SaveDB() создаст именно эту запись. Отметка изменения
    // обязательна: запись создана до загрузки и иначе считалась бы уже сохранённой
    if (sqlite3_step(stmt) != SQLITE_ROW)
        NDT::MarkUpdated(lab.labInfo);
    else
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
