#include "methodsNdt.hpp"

std::string GetMethodAbbreviation(Method value)
{
    std::string result;
    switch (value)
    {
    case Method::VT:
        result = "ВИК";
        break;
    case Method::RT:
        result = "РК";
        break;
    case Method::UT:
        result = "УК";
        break;
    case Method::DRT:
        result = "ЦРК";
        break;
    case Method::PT:
        result = "ПВК";
        break;
    case Method::LT:
        result = "ПВТ";
        break;
    case Method::MT:
        result = "МК";
        break;
    case Method::DT:
        result = "Расслоение";
        break;
    case Method::ECT:
        result = "ВК";
        break;
    case Method::UTM:
        result = "УТ";
        break;
    }
    return result;
}

std::string GetCategoryStr(Category value)
{
    std::string result;

    switch (value)
    {
    case Category::H:
        result = "В";
        break;
    case Category::I:
        result = "I";
        break;
    case Category::II:
        result = "II";
        break;
    case Category::III:
        result = "III";
        break;
    case Category::IV:
        result = "IV";
        break;
    }
    return result;
}

std::string GetWeldJointTypeStr(WeldJointType value)
{
    std::string result;

    switch (value)
    {
    case WeldJointType::Butt:
        result = "Стыковое";
        break;
    case WeldJointType::Corner:
        result = "Угловое";
        break;
    case WeldJointType::Tjoint:
        result = "Тавровое";
        break;
    case WeldJointType::Lap:
        result = "Нахлёсточное";
        break;
    }
    return result;
}
