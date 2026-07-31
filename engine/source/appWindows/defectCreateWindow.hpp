#pragma once

#include "imgui.h"

class ReportData;
class DefRGC;

class DefectCreateWindow
{
public:
    void Show(ReportData &report, bool &isOpen);

private:
    void ConstructDefectRGCString(DefRGC &input);
    void DefectTable(ReportData &report);

    int defectTableRows = 0;

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
};