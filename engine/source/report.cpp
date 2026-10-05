#include "report.hpp"

#include <cmath>
#include <algorithm>
#include <format>
#include <utility>
#include "laboratory.hpp"

namespace
{
/// @brief Размер для записи дефекта: десятичный разделитель - запятая,
/// ",0" пишется только у однозначных размеров - "1,0", но "15"
std::string FormatDefectSize(float value)
{
    const long tenths = std::lround(value * 10.f);
    const long whole = tenths / 10;
    const long fraction = tenths % 10;

    if (fraction == 0 && whole >= 10)
        return std::to_string(whole);

    return std::format("{:d},{:d}", whole, fraction);
}

/// @brief Одинаковы ли размеры в записи: сравниваем в целых десятых, а не float напрямую
bool IsSameDefectSize(float a, float b)
{
    return std::lround(a * 10.f) == std::lround(b * 10.f);
}

/// @brief Одинаковы ли дефекты в записи заключения: сравниваются только печатаемые поля
bool IsSameDefectRecord(const DefectRt &a, const DefectRt &b)
{
    if (a.symbol != b.symbol)
        return false;

    const DefectRtFields fields = GetDefectRtFields(a.symbol);
    if (fields.length && !IsSameDefectSize(a.length, b.length))
        return false;
    if (fields.size && (!IsSameDefectSize(a.width, b.width) || !IsSameDefectSize(a.height, b.height)))
        return false;
    if (fields.sign && a.endGreaterThan != b.endGreaterThan)
        return false;

    return true;
}
}

DefectRtFields GetDefectRtFields(DefectRtSymbol symbol)
{
    DefectRtFields result;
    switch (symbol)
    {
    case DefectRtSymbol::Aa: /// одиночные: Ba1,5×1,5≤
    case DefectRtSymbol::Ak:
    case DefectRtSymbol::Ba:
    case DefectRtSymbol::Da:
    case DefectRtSymbol::Dc:
    case DefectRtSymbol::Bd:
    case DefectRtSymbol::Fc2:
        result.size = true;
        result.sign = true;
        break;
    case DefectRtSymbol::Ac: /// скопления и цепочки: Ac25-2,0×1,0≤
    case DefectRtSymbol::Bc:
    case DefectRtSymbol::Ab:
    case DefectRtSymbol::Bb:
        result.length = true;
        result.size = true;
        result.sign = true;
        break;
    case DefectRtSymbol::Fa: /// протяжённые: Fa15>
    case DefectRtSymbol::Fb:
    case DefectRtSymbol::Fe:
        result.length = true;
        result.sign = true;
        break;
    case DefectRtSymbol::E: /// E12
    case DefectRtSymbol::Mw:
        result.length = true;
        break;
    case DefectRtSymbol::Fc1: /// только обозначение
    case DefectRtSymbol::Fd:
    case DefectRtSymbol::delta1:
    case DefectRtSymbol::delta2:
    case DefectRtSymbol::Count:
        break;
    }
    return result;
}

float RoundDefectSize(float value)
{
    if (value <= 0.f)
        return 0.f;

    constexpr float eps = 1e-3f; ///< погрешность float: 1.5f / 0.1f чуть больше 15, без поправки ceil дал бы 16
    const float step = value <= 3.f + eps ? 0.1f : 0.5f;
    return std::ceil(value / step - eps) * step;
}

Report::Report()
{
    controlDate = NDT::GetCurrentIsoDate();
    reportDate = controlDate;
    methodValue = Method::VT;
    methodHeader = GetMethodReportTitle(methodValue);
    pipeCategory = Category::H;
    weldType = WeldJointType::Butt;
    sectionType1 = SectionType::SingleSeam;
    sectionType2 = SectionType::SingleSeam;
    weldingMethods = {WeldingMethod::RD};
    exposureScheme = ExposureScheme::Panoramic;
    perimeter = static_cast<int>(std::lround(diameter * 3.141592f));
    metalOptDenMax = NDT::GetMetalDensity(negatoscopeBrightness); /// иначе до правки яркости предел остаётся мусором
}

std::optional<int> Report::GetMinSeamDistance() const
{
    const int seamCount1 = GetSeamCount(sectionType1);
    const int seamCount2 = GetSeamCount(sectionType2);

    if (seamCount1 == 0 || seamCount2 == 0 || perimeter <= 0)
        return std::nullopt;

    const std::array<int, 2> coords1{coordSec1Weld1, coordSec1Weld2};
    const std::array<int, 2> coords2{coordSec2Weld1, coordSec2Weld2};

    int result = perimeter;
    for (int i = 0; i < seamCount1; ++i)
    {
        for (int j = 0; j < seamCount2; ++j)
        {
            /// швы лежат на окружности стыка: расстояние по одной дуге равно разнице координат,
            /// по встречной - остатку периметра; берём короткую
            const int delta = std::abs(coords1.at(static_cast<size_t>(i)) - coords2.at(static_cast<size_t>(j)));
            result = std::min(result, std::min(delta, perimeter - delta));
        }
    }

    return result;
}

std::string Report::GetDefectRecord(const DefectRt &defect, int count) const
{
    const DefectRtFields fields = GetDefectRtFields(defect.symbol);
    std::string result;

    if (!defect.acceptable && exposureScheme != ExposureScheme::Ellipse) /// на эллипс координата не вводится
        result += std::format("({:d}) ", defect.coord);

    if (count > 1)
        result += std::to_string(count);

    result += GetDefectRTName(defect.symbol);

    if (fields.length)
    {
        result += FormatDefectSize(defect.length);
        if (fields.size)
            result += "-";
    }

    if (fields.size)
        result += FormatDefectSize(defect.width) + "×" + FormatDefectSize(defect.height);

    if (fields.sign)
        result += defect.endGreaterThan ? ">" : "≤";

    return result;
}

int Report::GetLengthSectionCount() const
{
    constexpr int eps = 2; ///< как в NDT::CalculateNumString: остаток в пару миллиметров не даёт лишнего участка
    return std::max(1, (perimeter - eps + sectionLength - 1) / sectionLength);
}

bool Report::IsSplitByFilms() const
{
    return exposureScheme != ExposureScheme::Ellipse &&
           GetLengthSectionCount() <= GetFilmMeasurementCount(exposureScheme);
}

int Report::GetSectionCount() const
{
    if (exposureScheme == ExposureScheme::Ellipse)
        return ellipseExposureCount;

    if (IsSplitByFilms())
        return GetFilmMeasurementCount(exposureScheme);

    return GetLengthSectionCount();
}

std::string Report::GetSectionTitle() const
{
    if (exposureScheme == ExposureScheme::Ellipse)
        return "Экспозиция";

    if (IsSplitByFilms())
        return "Снимок";

    return "Участок";
}

int Report::GetSectionStart(int section) const
{
    if (IsSplitByFilms())
        return perimeter * section / GetSectionCount();

    return section * sectionLength;
}

int Report::GetSection(int coord) const
{
    /// по снимкам: обратное к GetSectionStart - координата попадает в долю, начало которой не дальше её
    const int section = IsSplitByFilms() ? coord * GetSectionCount() / std::max(perimeter, 1) : coord / sectionLength;
    return std::clamp(section, 0, GetSectionCount() - 1);
}

int Report::GetDefectSection(const DefectRt &defect) const
{
    if (exposureScheme == ExposureScheme::Ellipse)
        return std::clamp(defect.exposure, 0, GetSectionCount() - 1);

    return GetSection(defect.coord);
}

int Report::GetMeasurementCount() const
{
    if (exposureScheme == ExposureScheme::Ellipse)
        return ellipseExposureCount;

    return GetFilmMeasurementCount(exposureScheme);
}

void Report::SetDefaultMeasurementCoords()
{
    const int count = static_cast<int>(filmMeasurements.size());

    /// центр i-й доли: perimeter * (i + 0.5) / count - в целых, чтобы не тянуть float ради координаты в мм
    for (int i = 0; i < count; ++i)
        filmMeasurements.at(static_cast<size_t>(i)).coord = perimeter * (2 * i + 1) / (2 * count);
}

std::string Report::GetSectionRangeStr(int section) const
{
    if (exposureScheme == ExposureScheme::Ellipse)
        return std::format("{:d} экспозиция", section + 1);

    const int start = GetSectionStart(section);
    const int end = section == GetSectionCount() - 1 ? 0 : GetSectionStart(section + 1); /// шов замкнут: последний участок кончается в нуле

    if (IsSplitByFilms())
        return std::format("{:d} ({:d}-{:d})", section + 1, start, end); /// номер снимка и его участок - коротко, чтобы влезло в ячейку бланка

    return std::format("{:d}-{:d}", start, end);
}

std::string Report::GetSectionDefectsStr(int section) const
{
    /// дефекты участка по порядку координат - в том же порядке пойдут записи
    std::vector<const DefectRt *> defects;
    for (const DefectRt &defect : defRGCList)
        if (GetDefectSection(defect) == section)
            defects.push_back(&defect);

    std::sort(defects.begin(), defects.end(), [](const DefectRt *a, const DefectRt *b)
              { return a->coord < b->coord; });

    /// первый дефект группы + сколько таких же нашлось; недопустимые не склеиваются ни с чем
    std::vector<std::pair<const DefectRt *, int>> groups;
    for (const DefectRt *defect : defects)
    {
        auto same = std::find_if(groups.begin(), groups.end(), [&](const auto &group)
                                 { return defect->acceptable && group.first->acceptable &&
                                          IsSameDefectRecord(*group.first, *defect); });

        if (same != groups.end())
            ++same->second;
        else
            groups.emplace_back(defect, 1);
    }

    std::string result;
    for (const auto &[defect, count] : groups)
    {
        if (!result.empty())
            result += "; ";
        result += GetDefectRecord(*defect, count);
    }

    return result.empty() ? "-" : result;
}

bool Report::IsSectionAcceptable(int section) const
{
    return std::none_of(defRGCList.begin(), defRGCList.end(), [&](const DefectRt &defect)
                        { return !defect.acceptable && GetDefectSection(defect) == section; });
}

const FilmMeasurement *Report::GetSectionMeasurement(int section) const
{
    if (exposureScheme == ExposureScheme::Ellipse) /// на эллипс замер - по одному на экспозицию, по порядку
        return static_cast<size_t>(section) < filmMeasurements.size() ? &filmMeasurements.at(static_cast<size_t>(section)) : nullptr;

    for (const FilmMeasurement &measurement : filmMeasurements)
        if (GetSection(measurement.coord) == section)
            return &measurement;

    return nullptr;
}

std::string Report::GetMethodReportTitle(Method value) const
{
    std::string result;
    switch (value)
    {
    case Method::VT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ ВИЗУАЛЬНЫМ И ИЗМЕРИТЕЛЬНЫМ МЕТОДОМ";
        break;
    case Method::RT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ";
        break;
    case Method::UT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ УЛЬТРАЗВУКОВЫМ МЕТОДОМ";
        break;
    case Method::DRT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ (цифровой)";
        break;
    case Method::PT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ КАПИЛЛЯРНЫМ МЕТОДОМ";
        break;
    case Method::LT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ МЕТОДОМ ПВТ";
        break;
    case Method::MT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ МАГНИТОПОРОШКОВЫМ МЕТОДОМ";
        break;
    case Method::DT:
        result = "ПО КОНТРОЛЮ СПЛОШНОСТИ КРОМОК СВАРИВАЕМЫХ ЭЛЕМЕНТОВ УЛЬТРАЗВУКОВЫМ МЕТОДОМ";
        break;
    default:
        break;
    }
    return result;
}

std::string Report::GetDefectRTName(DefectRtSymbol value)
{
    std::string result;
    switch (value)
    {
    case DefectRtSymbol::Aa:
        result = "Aa";
        break;
    case DefectRtSymbol::Ak:
        result = "Ak";
        break;
    case DefectRtSymbol::Ba:
        result = "Ba";
        break;
    case DefectRtSymbol::Ac:
        result = "Ac";
        break;
    case DefectRtSymbol::Bc:
        result = "Bc";
        break;
    case DefectRtSymbol::Ab:
        result = "Ab";
        break;
    case DefectRtSymbol::Bb:
        result = "Bb";
        break;
    case DefectRtSymbol::Da:
        result = "Da";
        break;
    case DefectRtSymbol::Dc:
        result = "Dc";
        break;
    case DefectRtSymbol::Bd:
        result = "Bd";
        break;
    case DefectRtSymbol::Fc2:
        result = "Fc2";
        break;
    case DefectRtSymbol::E:
        result = "E";
        break;
    case DefectRtSymbol::Fa:
        result = "Fa";
        break;
    case DefectRtSymbol::Fb:
        result = "Fb";
        break;
    case DefectRtSymbol::Fe:
        result = "Fe";
        break;
    case DefectRtSymbol::delta1:
        result = "∆1";
        break;
    case DefectRtSymbol::delta2:
        result = "∆2";
        break;
    case DefectRtSymbol::Fc1:
        result = "Fc1";
        break;
    case DefectRtSymbol::Fd:
        result = "Fd";
        break;
    case DefectRtSymbol::Mw:
        result = "Mw";
        break;
    case DefectRtSymbol::Count: // служебный маркер, не является значением дефекта
        result = "";
        break;
    }
    return result;
}

DefectRtSymbol Report::ParseDefectRtSymbol(const std::string &name)
{
    for (int i = 0; i < static_cast<int>(DefectRtSymbol::Count); ++i)
    {
        auto value = static_cast<DefectRtSymbol>(i);
        if (GetDefectRTName(value) == name)
            return value;
    }

    return DefectRtSymbol::Aa;
}
