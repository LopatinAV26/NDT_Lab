#pragma once

#include <filesystem>
#include <vector>

struct sqlite3;
struct Employee;

class DatabaseManager
{
public:
    explicit DatabaseManager(const std::filesystem::path &pathToDb);
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    DatabaseManager(DatabaseManager &&) = delete;
    DatabaseManager &operator=(DatabaseManager &&) = delete;
    ~DatabaseManager();

    void SaveEmployees(const std::vector<Employee> &employees);
    std::vector<Employee> LoadEmployees();

private:
    sqlite3 *db = nullptr;
};