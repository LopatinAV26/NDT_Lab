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