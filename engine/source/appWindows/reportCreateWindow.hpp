#pragma once

#include "defectCreateWindow.hpp"

#include <vector>
#include "methodsNdt.hpp"

class Laboratory;
struct Employee;
struct Inspector;

class ReportCreateWindow
{
public:
    void Show(Report &report, bool &isOpen, Laboratory &lab);

private:
    /// @brief Вернуть список сотрудников, отфильтрованных по наличию действующего метода
    /// @param lists 
    /// @param method метод, по которому нужно фильтровать сотрудников
    /// @param reportDate дата в формате ISO (YYYY-MM-DD), на которую нужно проверить действителен ли допуск
    /// @return отфильтрованный список сотрудников
    std::vector<Employee> MethodFilter(const std::vector<Employee> &lists, const Method method, const std::string &reportDate);
    std::vector<Inspector> MethodFilter(const std::vector<Inspector> &lists, const Method method, const std::string &reportDate);

    bool defectWindowIsOpen = false;
    DefectCreateWindow defectCreateWindow;

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
    // ImGuiWindowFlags_NoSavedSettings;
};