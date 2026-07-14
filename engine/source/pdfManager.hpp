#pragma once

#include <string>
#include <array>
#include "podofo/podofo.h"

class ProtocolData;
class ApplicationData;

enum class FontStyle : uint8_t
{
    Regular = 0U,
    Bold,
    Italic,
    BoldItalic
};

struct CellStyle
{
    PoDoFo::PdfHorizontalAlignment hAlign = PoDoFo::PdfHorizontalAlignment::Left;
    PoDoFo::PdfVerticalAlignment vAlign = PoDoFo::PdfVerticalAlignment::Center;
    double fontSize = 8.0;
    FontStyle fontStyle = FontStyle::Regular;
};

struct Cell
{
    std::string strData;
    double x;
    double y;
    double w;
    double h;
    PoDoFo::PdfDrawTextMultiLineParams params;
    double fontSize;
    FontStyle fontStyle;
};

class PdfManager
{
public:
    PdfManager(ApplicationData &coreAppData);
    void CreateTableRGC(const ProtocolData &data);
    Cell CreateCell(PoDoFo::PdfPainter &painter,
                    std::string str,
                    double x, double y, double w, double h,
                    CellStyle style = {});

private:
    ApplicationData &appData;
    std::array<PoDoFo::PdfFont *, 4> fonts;
    // PoDoFo::PdfFont &GetFont(FontStyle style) const;
    PoDoFo::Rect MmToPt(double x_mm, double y_mm,
                        double w_mm, double h_mm);
};