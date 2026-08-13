#include "report.hpp"

Report::Report()
{
    controlDate = NDT::GetCurrentIsoDate();
    reportDate = controlDate;
    pipeCategory = pipeCategoryList.at(0);
    methodValue = Method::VT;
    method = GetMethodTitle(methodValue);
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
    case Method::MT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ МАГНИТОПОРОШКОВЫМ МЕТОДОМ";
        break;
    case Method::DT:
        result = "ПО КОНТРОЛЮ СПЛОШНОСТИ КРОМОК СВАРИВАЕМЫХ ЭЛЕМЕНТОВ УЛЬТРАЗВУКОВЫМ МЕТОДОМ";
        break;
    /* case Method::LT:
        result = "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ МЕТОДОМ ПВТ";
        break; */
    default:
        break;
    }
    return result;
}
