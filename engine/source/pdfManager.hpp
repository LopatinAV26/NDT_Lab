#pragma once

#include <string>
#include <array>
#include <vector>
#include "podofo/podofo.h"

class ApplicationData;

enum class FontStyle : uint8_t
{
	Regular,
	Bold,
	Italic,
	BoldItalic
};

enum class NDTColor : uint8_t
{
	White,
	Black,
	Red,
	Green,
	Blue
};

struct CellStyle
{
	double width = 5.0;															  ///< ширина ячейки, мм
	std::string text{};															  ///< текст в ячейке
	double fontSize = 8.0;														  ///< размер шрифта, pt
	PoDoFo::PdfHorizontalAlignment hAlign = PoDoFo::PdfHorizontalAlignment::Left; ///< горизонтальное выравнивание
	PoDoFo::PdfVerticalAlignment vAlign = PoDoFo::PdfVerticalAlignment::Center;	  ///< вертикальное выравнивание
	FontStyle fontStyle = FontStyle::Regular;									  ///< начертание шрифта
	bool isRectVisible = true;
	PoDoFo::PdfColor fillColor = PoDoFo::PdfColor(1.0);		///< цвет заливки ячейки, по умолчанию белый
	PoDoFo::PdfColor textColor = PoDoFo::PdfColor(0.0);		///< цвет текста ячейки, по умолчанию черный
	PoDoFo::PdfColor strokingColor = PoDoFo::PdfColor(0.0); ///< цвет обводки ячейки, по умолчанию черный
};

struct Cell
{
	std::string text;
	double x;
	double y;
	double w;
	double h;
};

class PdfManager
{
public:
	explicit PdfManager(ApplicationData &coreAppData);

	/**
	 * @brief Создаёт новую страницу, вызов в начале рисования на новой странице обязателен
	 * */
	void NewPage();
	void SaveDocument();
	Cell CreateCell(double x, double y, double h, CellStyle cStyle);
	std::vector<Cell> CreateRow(double height, std::initializer_list<CellStyle> cells);
	PoDoFo::PdfColor Color(NDTColor c) const;

	const double topIndent = 20.0;	   // верхний отступ от края страницы, мм
	const double bottomIndent = 10.0;  // нижний отступ, мм
	const double leftIndent = 5.0;	   // левый отступ, мм
	const double rightIndent = 5.0;	   // правый отступ, мм
	double yEnd;
	double cursorRowY = 0.0; // начало рисования следующей строки таблицы по Y, увеличивается на высоту строки поле каждого вызова CreateRow

private:
	PoDoFo::Rect MmToPt(double x_mm, double y_mm, double w_mm, double h_mm);

	ApplicationData &appData;
	std::array<PoDoFo::PdfFont *, 4> fonts;

	PoDoFo::PdfMemDocument doc;
	PoDoFo::Rect pageRect;
	PoDoFo::PdfPainter painter;

	double lineWidth = 0.1;			   // толщина рисуемых линий, единицы PDF
	const double textRectIndent = 1.0; // отступ текста от внутренних границ ячейки, мм
	double xEnd;					   // правая граница рисования, мм
	double yStart;
	double dX;				 // Граница, почти вплотную к правому полю, рисование после которой возвращает ошибку, мм
	double mmToPt;			 // коэффициент для перевода из единиц PDF в миллиметры
	bool isLandscape = true; // Флаг, указывающий на то, что страница в альбомной ориентации
};