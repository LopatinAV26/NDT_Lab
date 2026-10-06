#pragma once

#include "imgui.h"
#include "report.hpp"

//class Report;
//struct DefRT;

class DefectCreateWindow
{
public:
    /// @param isUnsaved у заключения есть правки, не записанные в базу - точка в заголовке окна
    void Show(Report &report, bool &isOpen, bool isUnsaved);

private:
    ImGuiWindowFlags window_flags =
        // ImGuiWindowFlags_NoDecoration |
        // ImGuiWindowFlags_NoTitleBar |
        // ImGuiWindowFlags_NoMove |
        //ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;
};