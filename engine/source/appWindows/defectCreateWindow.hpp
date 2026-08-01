#pragma once

#include "imgui.h"

class ReportData;
class DefRT;

class DefectCreateWindow
{
public:
    void Show(ReportData &report, bool &isOpen);

private:
    void ConstructDefectRGCString(DefRT &input);
    void DefectTable(ReportData &report);

    int defectTableRows = 0;

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
};