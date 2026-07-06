#pragma once

#include <string>
#include "podofo/podofo.h"


struct Cell
{
    std::string name;
    int x;
    int y;
    int w;
    int h;
    
};

class PdfManager
{
public:
    void CreateTable();

private:
};