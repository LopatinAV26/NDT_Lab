#pragma once

#include "imgui.h"

class Report;
class DefRT;

class DefectCreateWindow
{
public:
    void ShowDefectCreateWindow(Report &report, bool &isOpen);

private:
    void ConstructDefectRGCString(DefRT &input);

    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        //ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
};