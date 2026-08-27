#include "report.hpp"

#include <cmath>
#include "laboratory.hpp"

Report::Report()
{
    controlDate = NDT::GetCurrentIsoDate();
    reportDate = controlDate;
    pipeCategory = pipeCategoryList.at(0);
    methodValue = Method::VT;
    method = GetMethodTitle(methodValue);
    perimeter = static_cast<int>(std::lround(diameter * 3.141592f));
}

std::string Report::GetMethodTitle(Method value) const
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

std::string Report::GetDefectRTName(DefectRT value) const
{
    std::string result;
    switch (value)
    {
    case DefectRT::Aa:
        result = "Aa";
        break;
    case DefectRT::Ak:
        result = "Ak";
        break;
    case DefectRT::Ba:
        result = "Ba";
        break;
    case DefectRT::Ac:
        result = "Ac";
        break;
    case DefectRT::Bc:
        result = "Bc";
        break;
    case DefectRT::Ab:
        result = "Ab";
        break;
    case DefectRT::Bb:
        result = "Bb";
        break;
    case DefectRT::Da:
        result = "Da";
        break;
    case DefectRT::Dc:
        result = "Dc";
        break;
    case DefectRT::Bd:
        result = "Bd";
        break;
    case DefectRT::Fc2:
        result = "Fc2";
        break;
    case DefectRT::E:
        result = "E";
        break;
    case DefectRT::Fa:
        result = "Fa";
        break;
    case DefectRT::Fb:
        result = "Fb";
        break;
    case DefectRT::Fe:
        result = "Fe";
        break;
    case DefectRT::delta1:
        result = "∆1";
        break;
    case DefectRT::delta2:
        result = "∆2";
        break;
    case DefectRT::Fc1:
        result = "Fc1";
        break;
    case DefectRT::Fd:
        result = "Fd";
        break;
    case DefectRT::Mw:
        result = "Mw";
        break;
    default:
        result = "";
        break;
    }
    return result;
}
