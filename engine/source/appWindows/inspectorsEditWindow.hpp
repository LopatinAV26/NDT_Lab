#pragma once

#include "imgui.h"

struct Inspector;

class InspectorsEditWindow
{
public:
    void Show(Inspector &inspector, bool &isOpen);

private:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
};
