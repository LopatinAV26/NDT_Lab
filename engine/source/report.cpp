#include "report.hpp"

#include <cmath>
#include "laboratory.hpp"

Report::Report()
{
    controlDate = NDT::GetCurrentIsoDate();
    reportDate = controlDate;
    methodValue = Method::VT;
    methodHeader = GetMethodReportTitle(methodValue);
    pipeCategory = Category::H;
    perimeter = static_cast<int>(std::lround(diameter * 3.141592f));
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
