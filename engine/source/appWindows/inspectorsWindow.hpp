#pragma once

#include <vector>
#include "inspectorsEditWindow.hpp"

struct Inspector;

class InspectorsWindow
{
public:
    void Show(std::vector<Inspector> &inspectorsList);

private:
    bool inspectorEditWindowIsOpen = false;
    InspectorsEditWindow inspectorEditWindow;
};
