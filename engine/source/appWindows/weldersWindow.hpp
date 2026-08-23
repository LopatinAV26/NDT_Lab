#pragma once

#include <vector>
#include "weldersEditWindow.hpp"

struct Welder;

class WeldersWindow
{
public:
    void Show(std::vector<Welder> &weldersList);

private:
    bool welderEditWindowIsOpen = false;
    WeldersEditWindow welderEditWindow;
};
