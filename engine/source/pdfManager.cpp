#include "pdfManager.hpp"
#include <stdexcept>
#include <cmath>
#include <SDL3/SDL.h>
#include "applicationData.hpp"
#include "protocolData.hpp"

PdfManager::PdfManager(ApplicationData &coreAppData)
	: appData{coreAppData}
{
	pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, true);
	fonts.at(static_cast<uint8_t>(FontStyle::Regular)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontRegular.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Bold)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBold.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Italic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontItalic.string());
	fonts.at(static_cast<uint8_t>(FontStyle::BoldItalic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBoldItalic.string());
}

void PdfManager::CreateTableRGC(const ProtocolData &data)
{
	PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
	painter.SetCanvas(page);
	painter.GraphicsState.SetLineWidth(0.1);

	CreateRow(10, {{100, data.masterNameTitle},
				   {40, data.masterNameList.at(data.masterNameIndex)},
				   {80, data.masterOrganization + ", " + data.masterCertNumber},
				   {20},
				   {25, data.protocolDate}});

	painter.FinishDrawing();

	doc.Save("test.pdf");
}

Cell PdfManager::CreateCell(double x, double y, double w, double h,
							std::string str,
							double fontSize,
							FontStyle fontStyle,
							bool rect,
							PoDoFo::PdfHorizontalAlignment hAlign,
							PoDoFo::PdfVerticalAlignment vAlign,
							double indent)
{
	constexpr double mmToPt = 72.0 / 25.4;
	Cell cell;
	cell.strData = std::move(str);
	cell.x = x;
	cell.y = y;
	cell.w = w;
	cell.h = h;
	cell.fontSize = {fontSize};
	cell.fontStyle = fontStyle;
	cell.params.HorizontalAlignment = hAlign;
	cell.params.VerticalAlignment = vAlign;
	cell.indent = indent;
	painter.TextState.SetFont(*fonts.at(static_cast<uint8_t>(cell.fontStyle)), cell.fontSize);
	if (rect)
		painter.DrawRectangle(cell.x * mmToPt, cell.y * mmToPt, cell.w * mmToPt, cell.h * mmToPt);
	painter.DrawTextMultiLine(cell.strData, (cell.x + cell.indent) * mmToPt, cell.y * mmToPt, (cell.w - cell.indent * 2.0) * mmToPt, cell.h * mmToPt, cell.params);
	return cell;
}

void PdfManager::CreateRow(double height, std::initializer_list<CellStyle> cells)
{
	double cursorX = lateralIndent;
	for (const auto &cell : cells)
	{
		if (cursorX > hEnd - 3.0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Невозможно создать всю строку. Превышение ширины страницы. Уменьшите ширину ячеек");
			cursorY += height;
			return;
		}
		double width = cell.width;
		if (cursorX + width > hEnd)
			width = hEnd - cursorX; // Корректировка границы последней ячейки по правому краю, если её граница заходит дальше
		CreateCell(cursorX, cursorY, width, height, cell.text, cell.fontSize, cell.fontStyle, cell.rectVisible, cell.hAlign, cell.vAlign);
		cursorX += cell.width;
	}
	cursorY += height;
}

PoDoFo::Rect PdfManager::MmToPt(double x_mm, double y_mm, double w_mm, double h_mm)
{
	constexpr double mmToPt = 72.0 / 25.4;
	return PoDoFo::Rect(x_mm * mmToPt, y_mm * mmToPt,
						w_mm * mmToPt, h_mm * mmToPt);
}