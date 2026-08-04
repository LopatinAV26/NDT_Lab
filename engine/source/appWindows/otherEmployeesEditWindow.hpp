#pragma once

#include "imgui.h"

struct Inspector;

class OtherEmployeesEditWindow
{
public:
    void Show(Inspector &inspector, bool &isOpen);

private:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
};
