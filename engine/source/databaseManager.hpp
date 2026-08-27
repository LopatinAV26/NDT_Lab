#pragma once

#include <filesystem>
#include <vector>

struct sqlite3;
struct Employee;
struct Inspector;
struct Master;
struct Welder;
struct Equipment;
struct ControlMap;
struct NormativeDocument;
class Laboratory;

class DatabaseManager
{
public:
    explicit DatabaseManager(const std::filesystem::path &pathToDb);
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    DatabaseManager(DatabaseManager &&) = delete;
    DatabaseManager &operator=(DatabaseManager &&) = delete;
    ~DatabaseManager();

    void SaveLaboratoryInfo(const Laboratory &lab);
    void LoadLaboratoryInfo(Laboratory &lab);

    void SaveEmployees(const std::vector<Employee> &employees);
    std::vector<Employee> LoadEmployees();

    void SaveInspectors(const std::vector<Inspector> &inspectors);
    std::vector<Inspector> LoadInspectors();

    void SaveMasters(const std::vector<Master> &masters);
    std::vector<Master> LoadMasters();

    void SaveWelders(const std::vector<Welder> &welders);
    std::vector<Welder> LoadWelders();

    void SaveEquipment(const std::vector<Equipment> &equipmentList);
    std::vector<Equipment> LoadEquipment();

    void SaveControlMaps(const std::vector<ControlMap> &controlMaps);
    std::vector<ControlMap> LoadControlMaps();

    void SaveNormativeDocuments(const std::vector<NormativeDocument> &normativeDocuments);
    std::vector<NormativeDocument> LoadNormativeDocuments();

private:
    /// @brief Создать таблицу laboratory_info, если её ещё нет, и дозаполнить отсутствующие
    /// колонки в уже существующей таблице (ALTER TABLE ADD COLUMN). Таблица хранит одну запись -
    /// SaveLaboratoryInfo/LoadLaboratoryInfo работают с ней через тот же upsert-по-id, что и
    /// Employee/Inspector, поэтому id лаборатории должен сохраняться стабильным между запусками
    void EnsureLaboratoryInfoTable();

    /// @brief Создать таблицу employees, если её ещё нет, и дозаполнить отсутствующие колонки
    /// в уже существующей таблице (ALTER TABLE ADD COLUMN) - на случай, если Employee получил
    /// новое поле после того, как база уже была создана на диске старой версией приложения
    void EnsureEmployeesTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы inspectors
    void EnsureInspectorsTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы masters
    void EnsureMastersTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы welders
    void EnsureWeldersTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы equipment
    void EnsureEquipmentTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы control_maps
    void EnsureControlMapsTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы normative_documents
    void EnsureNormativeDocumentsTable();

    sqlite3 *db = nullptr;
};