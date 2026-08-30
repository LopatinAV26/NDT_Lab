#pragma once

#include "imgui.h"
#include "report.hpp"

//class Report;
//struct DefRT;

class DefectCreateWindow
{
public:
    void Show(Report &report, bool &isOpen);

private:
    void ConstructDefectRT(const Report &report, DefectRt &input);

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        //ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
};