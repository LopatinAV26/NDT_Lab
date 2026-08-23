#pragma once

#include <vector>
#include "mastersEditWindow.hpp"

struct Master;

class MastersWindow
{
public:
    void Show(std::vector<Master> &mastersList);

private:
    bool masterEditWindowIsOpen = false;
    MastersEditWindow masterEditWindow;
};
