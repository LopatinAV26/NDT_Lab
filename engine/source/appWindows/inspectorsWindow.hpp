#pragma once

#include <vector>
#include "imgui.h"

struct Inspector;

class InspectorsWindow
{
public:
    void Show(std::vector<Inspector> &inspectorsList);

private:
    void Edit(Inspector &inspector, bool &isOpen);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
