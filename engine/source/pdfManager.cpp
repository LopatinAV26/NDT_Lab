#include "pdfManager.hpp"
#include <SDL3/SDL.h>
#include "applicationData.hpp"

PdfManager::PdfManager(ApplicationData &coreAppData)
	: appData{coreAppData}
{
	pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, isLandscape);
	fonts.at(static_cast<uint8_t>(FontStyle::Regular)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontRegular.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Bold)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBold.string());
	fonts.at(static_cast<uint8_t>(FontStyle::Italic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontItalic.string());
	fonts.at(static_cast<uint8_t>(FontStyle::BoldItalic)) = &doc.GetFonts().GetOrCreateFont(appData.pdfFontBoldItalic.string());

	mmToPt = 72.0 / 25.4;
	xEnd = pageRect.Width / mmToPt - rightIndent;
	yStart = pageRect.Height / mmToPt - topIndent;
	dX = xEnd - 2.0;
}

void PdfManager::NewPage()
{
	PoDoFo::PdfPage &page = doc.GetPages().CreatePage(pageRect);
	painter.SetCanvas(page);
	painter.GraphicsState.SetLineWidth(lineWidth);
	cursorRowY = 0.0;
}

void PdfManager::SaveDocument()
{
	painter.FinishDrawing();
	doc.Save("test.pdf");
}

Cell PdfManager::CreateCell(double x, double y, double h, CellStyle cStyle)
{
	Cell cell;
	cell.text = std::move(cStyle.text);
	cell.x = x + leftIndent;
	cell.y = yStart - y - h;
	cell.w = (cStyle.width == 0.0) ? xEnd - cell.x : cStyle.width;

	if (cell.x + cell.w > xEnd)
	{
		double delta = xEnd - cell.x - cell.w;
		cell.w = xEnd - cell.x;
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Ячейка была обрезана по правому отступу на %.1f мм", delta);
		cStyle.fillColor = Color(NDTColor::Red);
	}
	cell.h = h;

	painter.Save();

	painter.GraphicsState.SetNonStrokingColor(PoDoFo::PdfColor(cStyle.fillColor));	// предустановка цвета заливки
	painter.GraphicsState.SetStrokingColor(PoDoFo::PdfColor(cStyle.strokingColor)); // цвет рамки таблицы
	painter.TextState.SetFont(*fonts.at(static_cast<uint8_t>(cStyle.fontStyle)), cStyle.fontSize);

	if (cStyle.isRectVisible)
		painter.DrawRectangle(MmToPt(cell.x, cell.y, cell.w, cell.h), PoDoFo::PdfPathDrawMode::StrokeFill);

	painter.GraphicsState.SetNonStrokingColor(PoDoFo::PdfColor(cStyle.textColor)); // вернуть цвет текста перед рисованием текста(он же цвет заливки)
	painter.DrawTextMultiLine(cell.text, MmToPt(cell.x + textRectIndent, cell.y, cell.w - textRectIndent * 2.0, cell.h),
							  {.HorizontalAlignment = cStyle.hAlign, .VerticalAlignment = cStyle.vAlign});

	painter.Restore();

	return cell;
}

void PdfManager::CreateRow(double height, std::initializer_list<CellStyle> cells)
{
	double cursorX = 0.0;
	for (auto cell : cells)
	{
		if (cursorX >= dX)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Невозможно создать все ячейки. Превышение границы отступа. Уменьшите ширину ячеек");
			cursorRowY += height;
			return;
		}

		Cell drawn = CreateCell(cursorX, cursorRowY, height, cell);
		cursorX += drawn.w;
	}
	cursorRowY += height;
}

PoDoFo::PdfColor PdfManager::Color(NDTColor c) const
{
	switch (c)
	{
	case NDTColor::White:
		return PoDoFo::PdfColor(1.0);
	case NDTColor::Black:
		return PoDoFo::PdfColor(0.0);
	case NDTColor::Red:
		return PoDoFo::PdfColor(1.0, 0.0, 0.0);
	case NDTColor::Green:
		return PoDoFo::PdfColor(0.0, 1.0, 0.0);
	case NDTColor::Blue:
		return PoDoFo::PdfColor(0.0, 0.0, 1.0);
	default:
		return PoDoFo::PdfColor(0.0);
	}
}

PoDoFo::Rect PdfManager::MmToPt(double x_mm, double y_mm, double w_mm, double h_mm)
{
	return PoDoFo::Rect(x_mm * mmToPt, y_mm * mmToPt,
						w_mm * mmToPt, h_mm * mmToPt);
}