#pragma once

#include <vector>
#include "equipmentEditWindow.hpp"

struct Equipment;

class EquipmentWindow
{
public:
    void Show(std::vector<Equipment> &equipmentList);

private:
    bool equipmentEditWindowIsOpen = false;
    EquipmentEditWindow equipmentEditWindow;
};
