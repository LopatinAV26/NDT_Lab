#include "pdfManager.hpp"
#include <SDL3/SDL.h>
// #include "embeddedFonts.hpp"
#include "NotoSansRegular.cpp"
#include "NotoSansBold.cpp"
#include "NotoSansItalic.cpp"
#include "NotoSansBoldItalic.cpp"

PdfManager::PdfManager()
{
	pageRect = PoDoFo::PdfPage::CreateStandardPageSize(PoDoFo::PdfPageSize::A4, isLandscape);
	fonts.at(static_cast<uint8_t>(FontStyle::Regular)) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansRegular_data), NotoSansRegular_data_len));
	fonts.at(static_cast<uint8_t>(FontStyle::Bold)) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBold_data), NotoSansBold_data_len));
	fonts.at(static_cast<uint8_t>(FontStyle::Italic)) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansItalic_data), NotoSansItalic_data_len));
	fonts.at(static_cast<uint8_t>(FontStyle::BoldItalic)) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBoldItalic_data), NotoSansBoldItalic_data_len));

	mmToPt = 72.0 / 25.4;
	xEnd = pageRect.Width / mmToPt - rightIndent;
	yEnd = pageRect.Height / mmToPt - bottomIndent - topIndent;
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

void PdfManager::SaveDocument(std::string_view docName)
{
	painter.FinishDrawing();
	doc.Save(docName);
}

Cell PdfManager::TableCreateCell(double x, double y, double h, CellStyle cStyle)
{
	Cell cell;
	cell.text = std::move(cStyle.text);
	cell.x = x;
	double newX = {x + leftIndent};
	cell.y = y;
	double newY = {yStart - y - h};
	cell.w = {(cStyle.width == 0.0) ? xEnd - newX : cStyle.width};
	cell.h = h;

	if (newX + cell.w > xEnd)
	{
		double delta = xEnd - newX - cell.w;
		cell.w = xEnd - newX;
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Ячейка была обрезана по правому отступу на %.1f мм", delta);
		cStyle.fillColor = Color(NDTColor::Red);
	}

	painter.Save();

	painter.GraphicsState.SetNonStrokingColor(PoDoFo::PdfColor(cStyle.fillColor));	// предустановка цвета заливки
	painter.GraphicsState.SetStrokingColor(PoDoFo::PdfColor(cStyle.strokingColor)); // цвет рамки таблицы
	painter.TextState.SetFont(*fonts.at(static_cast<uint8_t>(cStyle.fontStyle)), cStyle.fontSize);

	if (cStyle.isRectVisible)
		painter.DrawRectangle(MmToPt(newX, newY, cell.w, cell.h), PoDoFo::PdfPathDrawMode::StrokeFill);

	painter.GraphicsState.SetNonStrokingColor(PoDoFo::PdfColor(cStyle.textColor)); // вернуть цвет текста перед рисованием текста(он же цвет заливки)
	painter.DrawTextMultiLine(cell.text, MmToPt(newX + textRectIndent, newY, cell.w - textRectIndent * 2.0, cell.h),
							  {.HorizontalAlignment = cStyle.hAlign, .VerticalAlignment = cStyle.vAlign});

	painter.Restore();

	return cell;
}

std::vector<Cell> PdfManager::TableCreateRow(double height, std::initializer_list<CellStyle> cells)
{
	std::vector<Cell> cellsVector;
	cellsVector.reserve(cells.size());
	for (const auto &cell : cells)
	{
		if (cursorX >= dX)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Невозможно создать все ячейки. Превышение границы отступа. Уменьшите ширину ячеек");
			cursorRowY += height;
			return cellsVector;
		}

		Cell drawn = TableCreateCell(cursorX, cursorRowY, height, cell);
		cursorX += drawn.w;
		cellsVector.push_back(drawn);
	}
	cursorRowY += height;

	return cellsVector;
}

void PdfManager::TableNewRow(double offset)
{
	cursorX = 0.0 + offset;
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