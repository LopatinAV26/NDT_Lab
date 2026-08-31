#pragma once

#include <vector>
#include "imgui.h"

struct Welder;

class WeldersWindow
{
public:
    void Show(std::vector<Welder> &weldersList);

private:
    void Edit(Welder &welder, bool &isOpen);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
