#pragma once

#include "protocol.hpp"

class DefectCreateWindow
{
public:
    void Show(ReportData &report, bool &isOpen);

private:
    void ConstructDefectRGCString(DefRGC &input);
    void DefectTable(ReportData &report);

    int defectTableRows = 0;
};