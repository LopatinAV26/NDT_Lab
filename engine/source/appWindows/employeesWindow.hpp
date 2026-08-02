#pragma once

#include <vector>
#include "employeeEditingWindow.hpp"

struct Employee;

class EmployeesWindow
{
public:
    void ShowEmployeesWindow(std::vector<Employee> &empl);

private:
    bool employeeEditWindowIsOpen = false;
    EmployeeEditingWindow emplEditWindow;

};