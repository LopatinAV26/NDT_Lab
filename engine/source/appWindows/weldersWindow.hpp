#pragma once

#include <vector>
#include "imgui.h"

struct Welder;

class Laboratory;

class WeldersWindow
{
public:
    void Show(std::vector<Welder> &weldersList, const Laboratory &lab);

private:
    void Edit(Welder &welder, bool &isOpen, bool isUnsaved);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
