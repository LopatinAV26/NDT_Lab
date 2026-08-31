#pragma once

#include <vector>
#include "imgui.h"

struct Employee;

class EmployeesWindow
{
public:
    void Show(std::vector<Employee> &empl);

private:
    void Edit(Employee &empl, bool &isOpen);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
