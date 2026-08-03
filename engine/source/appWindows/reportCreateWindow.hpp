#pragma once

#include "defectCreateWindow.hpp"

class Laboratory;

class ReportCreateWindow
{
public:
    void Show(Report &report, bool &isOpen, Laboratory &lab);

private:
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