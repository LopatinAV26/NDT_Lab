#pragma once

#include <vector>
#include "controlMapsEditWindow.hpp"

struct ControlMap;

class ControlMapsWindow
{
public:
    void Show(std::vector<ControlMap> &controlMapsList);

private:
    bool controlMapEditWindowIsOpen = false;
    ControlMapsEditWindow controlMapEditWindow;
};
