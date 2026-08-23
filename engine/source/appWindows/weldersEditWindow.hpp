#pragma once

#include "imgui.h"

struct Welder;

class WeldersEditWindow
{
public:
    void Show(Welder &welder, bool &isOpen);

private:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
};
