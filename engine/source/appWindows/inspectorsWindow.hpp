#pragma once

#include <vector>
#include "imgui.h"

struct Inspector;

class Laboratory;

class InspectorsWindow
{
public:
    void Show(std::vector<Inspector> &inspectorsList, const Laboratory &lab);

private:
    void Edit(Inspector &inspector, bool &isOpen, bool isUnsaved);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
