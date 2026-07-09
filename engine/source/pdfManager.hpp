#pragma once

#include <string>
#include "podofo/podofo.h"

class ProtocolData;

struct Cell
{
    std::string name;
    double x;
    double y;
    double w;
    double h;
};

class PdfManager
{
public:
    void CreateTableRGC(const ProtocolData& data);

private:
    double MmToPt(double num);
    PoDoFo::Rect MmToPt(double x_mm, double y_mm,
                        double w_mm, double h_mm);
};