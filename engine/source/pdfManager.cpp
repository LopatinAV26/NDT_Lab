#include "pdfManager.hpp"
#include <stdexcept>
#include <SDL3/SDL.h>
#include "applicationData.hpp"
#include "protocolData.hpp"

PdfManager::PdfManager(ApplicationData &coreAppData)
    : appData{coreAppData}
{
}

void PdfManager::CreateTableRGC(const ProtocolData &data)
{
    try
    {
        PoDoFo::PdfMemDocument doc;
        PoDoFo::Rect pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, true);
        PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
        fonts.at(static_cast<uint8_t>(FontStyle::Regular)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontRegular.string());
        fonts.at(static_cast<uint8_t>(FontStyle::Bold)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBold.string());
        fonts.at(static_cast<uint8_t>(FontStyle::Italic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontItalic.string());
        fonts.at(static_cast<uint8_t>(FontStyle::BoldItalic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBoldItalic.string());

        PoDoFo::PdfFont &fontRegular = doc.GetFonts().GetOrCreateFont(appData.pdfFontRegular.string());
        PoDoFo::PdfPainter painter;
        painter.SetCanvas(page);
        painter.TextState.SetFont(fontRegular, 9);
        painter.GraphicsState.SetLineWidth(0.25);

        Cell c1 = CreateCell(painter, data.masterNameTitle, 5, 5, 60, 10);
        Cell c2 = CreateCell(painter, data.masterNameList.at(data.masterNameIndex), c1.x + c1.w, c1.y, c1.w, c1.h);

        painter.FinishDrawing();
        doc.Save("Test document.pdf");
    }
    catch (const PoDoFo::PdfError &err)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to create PDF document: %s\n", err.what());
    }
}

Cell PdfManager::CreateCell(PoDoFo::PdfPainter &painter,
                            std::string str,
                            double x, double y, double w, double h,
                            CellStyle style)
{
    constexpr double mmToPt = 72.0 / 25.4;
    Cell cell;
    cell.strData = std::move(str);
    cell.x = x;
    cell.y = y;
    cell.w = w;
    cell.h = h;
    cell.params.HorizontalAlignment = style.hAlign;
    cell.params.VerticalAlignment = style.vAlign;
    cell.fontSize = {style.fontSize};
    cell.fontStyle = style.fontStyle;
    painter.TextState.SetFont(*fonts.at(static_cast<uint8_t>(cell.fontStyle)), cell.fontSize);
    painter.DrawRectangle(cell.x * mmToPt, cell.y * mmToPt, cell.w * mmToPt, cell.h * mmToPt);
    painter.DrawTextMultiLine(cell.strData, cell.x * mmToPt, cell.y * mmToPt, cell.w * mmToPt, cell.h * mmToPt, cell.params);
    return cell;
}

PoDoFo::Rect PdfManager::MmToPt(double x_mm, double y_mm, double w_mm, double h_mm)
{
    constexpr double mmToPt = 72.0 / 25.4;
    return PoDoFo::Rect(x_mm * mmToPt, y_mm * mmToPt,
                        w_mm * mmToPt, h_mm * mmToPt);
}