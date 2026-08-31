#pragma once

#include <vector>
#include "imgui.h"

struct Master;

class MastersWindow
{
public:
    void Show(std::vector<Master> &mastersList);

private:
    void Edit(Master &master, bool &isOpen);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    bool editWindow = false;
};
