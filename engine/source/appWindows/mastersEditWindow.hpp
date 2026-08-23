#pragma once

#include "imgui.h"

struct Master;

class MastersEditWindow
{
public:
    void Show(Master &master, bool &isOpen);

private:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
};
