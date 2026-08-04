#pragma once

#include <vector>
#include "otherEmployeesEditWindow.hpp"

struct Inspector;

class OtherEmployeesWindow
{
public:
    void Show(std::vector<Inspector> &inspectorsList);

private:
    bool inspectorEditWindowIsOpen = false;
    OtherEmployeesEditWindow inspectorEditWindow;
};
