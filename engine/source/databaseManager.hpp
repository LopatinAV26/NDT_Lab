#pragma once

#include <filesystem>
#include <vector>

struct sqlite3;
struct Employee;
struct Inspector;
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

    sqlite3 *db = nullptr;
};