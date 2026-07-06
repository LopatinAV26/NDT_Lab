#include "pdfManager.hpp"
#include <stdexcept>

void PdfManager::CreateTable()
{
    PoDoFo::PdfMemDocument doc;
    PoDoFo::Rect pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, true);
    PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
    PoDoFo::PdfPainter painter;
    painter.SetCanvas(page);
    PoDoFo::PdfFont *font = doc.GetFonts().SearchFont("Helvetica");
    if (font == nullptr)
        throw std::runtime_error("Invalid handle");
    painter.TextState.SetFont(*font, 12);

    

    doc.Save("Test document");
}
