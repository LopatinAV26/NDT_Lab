#include "pdfManager.hpp"
// #include <stdexcept>
//  #include <cmath>
#include <SDL3/SDL.h>
#include "applicationData.hpp"
// #include "protocolData.hpp"

PdfManager::PdfManager(ApplicationData &coreAppData)
	: appData{coreAppData}
{
	pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, isLandscape);
	fonts.at(static_cast<uint8_t>(FontStyle::Regular)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontRegular.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Bold)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBold.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Italic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontItalic.string());
	fonts.at(static_cast<uint8_t>(FontStyle::BoldItalic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBoldItalic.string());

	mmToPt = 72.0 / 25.4;
	hEnd = pageRect.Width / mmToPt - rightIndent;
	vEnd = pageRect.Height / mmToPt - topIndent;
	cursorY = bottomIndent;
	dX = hEnd - 2.0;
}

void PdfManager::StartTable()
{
	PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
	painter.SetCanvas(page);
	painter.GraphicsState.SetLineWidth(lineWidth);
}

void PdfManager::EndTable()
{
	painter.FinishDrawing();
	doc.Save("test.pdf");
}

Cell PdfManager::CreateCell(double x, double y, double h, CellStyle cStyle)
{
	Cell cell;
	cell.text = std::move(cStyle.text);
	cell.x = x;
	cell.y = y;
	cell.w = cStyle.width;
	cell.h = h;
	cell.fontSize = cStyle.fontSize;
	cell.fontStyle = cStyle.fontStyle;
	cell.params.HorizontalAlignment = cStyle.hAlign;
	cell.params.VerticalAlignment = cStyle.vAlign;
	cell.rectVisible = cStyle.rectVisible;
	painter.TextState.SetFont(*fonts.at(static_cast<uint8_t>(cell.fontStyle)), cell.fontSize);
	if (cStyle.rectVisible)
		painter.DrawRectangle(MmToPt(cell.x, cell.y, cell.w, cell.h));
	painter.DrawTextMultiLine(cell.text, MmToPt(cell.x + textRectIndent, cell.y, cell.w - textRectIndent * 2.0, cell.h), cell.params);
	return cell;
}

void PdfManager::CreateRow(double height, std::initializer_list<CellStyle> cells)
{
	double cursorX = leftIndent;
	for (auto cell : cells)
	{
		if (cursorX >= dX)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Невозможно создать все ячейки. Превышение ширины страницы. Уменьшите ширину ячеек");
			return;
		}
		if (cursorX + cell.width > hEnd)
			cell.width = hEnd - cursorX; // Корректировка границы последней ячейки по правому краю, если её граница заходит дальше
		CreateCell(cursorX, cursorY, height, cell);
		cursorX += cell.width;
	}
	cursorY += height;
}

PoDoFo::Rect PdfManager::MmToPt(double x_mm, double y_mm, double w_mm, double h_mm)
{
	return PoDoFo::Rect(x_mm * mmToPt, y_mm * mmToPt,
						w_mm * mmToPt, h_mm * mmToPt);
}