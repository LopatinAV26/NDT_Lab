#pragma once

#include "imgui.h"

struct Equipment;

class EquipmentEditWindow
{
public:
    void Show(Equipment &equipment, bool &isOpen);

private:
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
};
