#pragma once

#include "laboratory.hpp"
#include "defectCreateWindow.hpp"

class ReportCreateWindow
{
public:
    void Show(ReportData &report, bool &isOpen);
    bool defectWindowIsOpen = false;

    DefectCreateWindow defectCreateWindow;
};