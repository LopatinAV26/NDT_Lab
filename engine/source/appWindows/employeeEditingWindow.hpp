#pragma once

#include "imgui.h"

struct Employee;

class EmployeeEditingWindow
{
public:
    void Show(Employee &empl, bool &isOpen);

private:
    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        //ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
    // ImGuiWindowFlags_NoSavedSettings;
};