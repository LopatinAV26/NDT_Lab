#pragma once

#include "defectCreateWindow.hpp"

struct ReportData;

class ReportCreateWindow
{
public:
    void Show(ReportData &report, bool &isOpen);
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