#include "pdfManager.hpp"
#include <stdexcept>
#include "protocolData.hpp"

void PdfManager::CreateTableRGC(const ProtocolData &data)
{
    PoDoFo::PdfMemDocument doc;
    PoDoFo::Rect pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, true);
    PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
    PoDoFo::PdfFont *font = doc.GetFonts().SearchFont("FreeSans");
    if (font == nullptr)
        throw std::runtime_error("Invalid handle");
    PoDoFo::PdfPainter painter;
    painter.SetCanvas(page);
    painter.TextState.SetFont(*font, 12);
    // painter.GraphicsState.SetLineWidth(1.0);
    
    Cell cell1{.x = 5, .y = 5, .w = 100, .h = 10};
    painter.DrawRectangle(MmToPt(5, 5, 100, 10));
    PoDoFo::PdfDrawTextMultiLineParams params;
    params.HorizontalAlignment = PoDoFo::PdfHorizontalAlignment::Center; // Left / Center / Right
    params.VerticalAlignment = PoDoFo::PdfVerticalAlignment::Center;     // Top / Center / Bottom
    painter.DrawTextMultiLine(data.nameLab, MmToPt(5, 5, 100, 10), params);

    painter.FinishDrawing();
    doc.Save("Test document");
}

double PdfManager::MmToPt(double num)
{
    constexpr double mmToPt = 72.0 / 25.4;
    return num * mmToPt;
}

PoDoFo::Rect PdfManager::MmToPt(double x_mm, double y_mm, double w_mm, double h_mm)
{
    constexpr double mmToPt = 72.0 / 25.4;
    return PoDoFo::Rect(x_mm * mmToPt, y_mm * mmToPt,
                        w_mm * mmToPt, h_mm * mmToPt);
}