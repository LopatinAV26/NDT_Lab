#pragma once

#include <string>
#include <array>
#include "podofo/podofo.h"

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
    double width = 5.0;                                                           ///< ширина ячейки, мм
    std::string text{};                                                           ///< текст в ячейке
    double fontSize = 8.0;                                                        ///< размер шрифта, pt
    FontStyle fontStyle = FontStyle::Regular;                                     ///< начертание шрифта
    PoDoFo::PdfHorizontalAlignment hAlign = PoDoFo::PdfHorizontalAlignment::Left; ///< горизонтальное выравнивание
    PoDoFo::PdfVerticalAlignment vAlign = PoDoFo::PdfVerticalAlignment::Center;   ///< вертикальное выравнивание
    bool rectVisible = true;                                                      ///< рисовать ли обводку ячейки
};

struct Cell
{
    std::string text;
    double x;
    double y;
    double w;
    double h;
    double fontSize;
    FontStyle fontStyle;
    PoDoFo::PdfDrawTextMultiLineParams params;
    bool rectVisible;
};

class PdfManager
{
public:
    explicit PdfManager(ApplicationData &coreAppData);

    void StartTable();
    
    void EndTable();

    /**
     * @brief Создаёт и рисует одну ячейку по заданным координатам и стилю.
     *
     * (x, y) — левый нижний угол ячейки в системе координат PDF (0,0 — низ
     * страницы, Y растёт вверх), в миллиметрах. Ширина, текст и оформление
     * берутся из @p cStyle (см. CellStyle) — поля задаются позиционно или
     * по имени по тем же правилам, что и у ячеек в CreateRow.
     *
     * Обычно отдельно не вызывается: используется как строительный блок
     * внутри CreateRow, которая сама вычисляет x/y для каждой ячейки строки
     * и корректирует ширину последней ячейки по правому краю страницы.
     * Прямой вызов имеет смысл для одиночной ячейки вне сетки строк.
     *
     * @code
     * CreateCell(5.0, 20.0, 10.0, {60.0, "Текст"});
     * CreateCell(5.0, 20.0, 10.0, {.width = 60.0, .text = "Текст", .rectVisible = false});
     * @endcode
     *
     * @param x      левая граница ячейки, мм
     * @param y      нижняя граница ячейки, мм (PDF: Y растёт вверх)
     * @param h      высота ячейки, мм
     * @param cStyle ширина, текст и оформление ячейки (см. CellStyle)
     * @return созданная Cell с итоговыми координатами и стилем
     */
    Cell CreateCell(double x, double y, double h, CellStyle cStyle);

    /**
     * @brief Создаёт строку таблицы из ячеек, идущих подряд слева направо.
     *
     * Каждый элемент @p cells — это CellStyle (width, text, fontSize, fontStyle,
     * rectVisible, hAlign, vAlign). Поля можно задать двумя способами,
     * не смешивая их в одной ячейке:
     *  - позиционно, только с начала и без пропусков: {ширина, текст, размер шрифта};
     *  - по имени: {.width = ..., .fontStyle = ...} — тогда можно указать
     *    любой поднабор полей, остальные останутся по умолчанию.
     *
     * @code
     * CreateRow(10, {
     *     {60.0, "Обычная ячейка"},                              // width + text, остальное по умолчанию
     *     {40.0, "С размером шрифта", 10.0},                     // + fontSize, позиционно
     *     {.width = 80.0, .text = "Жирным без рамки",
     *      .fontStyle = FontStyle::Bold, .rectVisible = false},  // по имени, пропуская fontSize
     * });
     * @endcode
     *
     * @param height высота строки, мм
     * @param cells  список ячеек строки (см. CellStyle)
     */
    void CreateRow(double height, std::initializer_list<CellStyle> cells);

private:
    PoDoFo::Rect MmToPt(double x_mm, double y_mm, double w_mm, double h_mm);

    ApplicationData &appData;
    std::array<PoDoFo::PdfFont *, 4> fonts;

    PoDoFo::PdfMemDocument doc;
    PoDoFo::Rect pageRect;
    PoDoFo::PdfPainter painter;

    double lineWidth = 0.1;            // толщина рисуемых линий, единицы PDF
    const double topIndent = 20.0;     // верхний отступ от края страницы, мм
    const double bottomIndent = 10.0;  // нижний отступ, мм
    const double leftIndent = 5.0;     // левый отступ, мм
    const double rightIndent = 5.0;    // правый отступ, мм
    const double textRectIndent = 1.0; // отступ текста от внутренних границ ячейки, мм
    double hEnd;                       // правая граница, мм
    double vEnd;                       // верхняя граница, мм
    double cursorY;                    // начало рисования следующей строки таблицы по Y, увеличивается на высоту строки поле каждого вызова CreateRow
    double dX;                         // Граница, почти вплотную к правому полю, рисование после которой возвращает ошибку, мм
    double mmToPt;                     // коэффициент для перевода из единиц PDF в миллиметры
    bool isLandscape = true;           // Флаг, указывающий на то, что страница в альбомной ориентации
};