#include "report.hpp"

#include <cmath>
#include <algorithm>
#include "laboratory.hpp"

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
    perimeter = static_cast<int>(std::lround(diameter * 3.141592f));
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

std::string Report::GetDefectRTName(DefectRtSymbol value) const
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

DefectRtSymbol Report::ParseDefectRtSymbol(const std::string &name) const
{
    for (int i = 0; i < static_cast<int>(DefectRtSymbol::Count); ++i)
    {
        auto value = static_cast<DefectRtSymbol>(i);
        if (GetDefectRTName(value) == name)
            return value;
    }

    return DefectRtSymbol::Aa;
}
