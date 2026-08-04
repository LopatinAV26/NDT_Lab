#pragma once

#include <vector>
#include "employeeEditingWindow.hpp"

struct Employee;

class EmployeesWindow
{
public:
    void Show(std::vector<Employee> &empl);

private:
    bool employeeEditWindowIsOpen = false;
    EmployeeEditingWindow emplEditWindow;

};