#pragma once

#include <vector>
#include "imgui.h"

struct Employee;

class Laboratory;

class EmployeesWindow
{
public:
    void Show(std::vector<Employee> &empl, const Laboratory &lab);

private:
    void Edit(Employee &empl, bool &isOpen, bool isUnsaved);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
