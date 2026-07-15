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
    double width = 5.0;
    std::string text{};
    double fontSize = 8.0;
    FontStyle fontStyle = FontStyle::Regular;
    bool rectVisible = true;
    PoDoFo::PdfHorizontalAlignment hAlign = PoDoFo::PdfHorizontalAlignment::Left;
    PoDoFo::PdfVerticalAlignment vAlign = PoDoFo::PdfVerticalAlignment::Center;
};

struct Cell
{
    std::string strData;
    double x;
    double y;
    double w;
    double h;
    double fontSize;
    FontStyle fontStyle;
    PoDoFo::PdfDrawTextMultiLineParams params;
    double indent;
};

class PdfManager
{
public:
    PdfManager(ApplicationData &coreAppData);

    void CreateTableRGC(const ProtocolData &data);
    Cell CreateCell(double x, double y, double w, double h,
                    std::string str = {},
                    double fontSize = 8.0,
                    FontStyle fontStyle = FontStyle::Regular,
                    bool rect = true,
                    PoDoFo::PdfHorizontalAlignment hAlign = PoDoFo::PdfHorizontalAlignment::Left,
                    PoDoFo::PdfVerticalAlignment vAlign = PoDoFo::PdfVerticalAlignment::Center,
                    double indent = 1.0);

    void CreateRow(double height = 5.0, std::initializer_list<CellStyle> cells = {});

private:
    PoDoFo::Rect MmToPt(double x_mm, double y_mm, double w_mm, double h_mm);

    ApplicationData &appData;
    std::array<PoDoFo::PdfFont *, 4> fonts;

    PoDoFo::PdfMemDocument doc;
    PoDoFo::Rect pageRect;
    PoDoFo::PdfPainter painter;

    const double topIndent = 20.0;
    const double bottomIndent = 10.0;
    const double lateralIndent = 5.0;
    const double hEnd = 297.0 - lateralIndent;
    const double vEnd = 210.0 - bottomIndent;
    double cursorY = bottomIndent;
};