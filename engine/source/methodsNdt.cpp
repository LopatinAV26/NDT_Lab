#include "methodsNdt.hpp"

/// разделитель обозначений в строке способов сварки: "РД+АПИ"
static constexpr char weldingMethodSeparator = '+';

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

WeldJointType ParseWeldJointType(const std::string &value)
{
    for (int i = 0; i < static_cast<int>(WeldJointType::Count); ++i)
    {
        auto item = static_cast<WeldJointType>(i);
        if (GetWeldJointTypeStr(item) == value)
            return item;
    }

    return WeldJointType::Butt;
}

std::string GetSectionTypeStr(SectionType value)
{
    std::string result;

    switch (value)
    {
    case SectionType::SingleSeam:
        result = "одношовная";
        break;
    case SectionType::DoubleSeam:
        result = "двухшовная";
        break;
    case SectionType::Seamless:
        result = "бесшовная";
        break;
    case SectionType::Transition:
        result = "переход";
        break;
    case SectionType::Flange:
        result = "фланец";
        break;
    }
    return result;
}

SectionType ParseSectionType(const std::string &value)
{
    for (int i = 0; i < static_cast<int>(SectionType::Count); ++i)
    {
        auto item = static_cast<SectionType>(i);
        if (GetSectionTypeStr(item) == value)
            return item;
    }

    return SectionType::SingleSeam;
}

int GetSeamCount(SectionType value)
{
    int result = 0;

    switch (value)
    {
    case SectionType::SingleSeam:
        result = 1;
        break;
    case SectionType::DoubleSeam:
        result = 2;
        break;
    case SectionType::Seamless:
        result = 0;
        break;
    case SectionType::Transition:
        /// штампосварные переходы имеют один продольный шов, кованые и точёные - ни одного;
        /// оставляем поле доступным, незаполненная координата в бланке остаётся нулём
        result = 1;
        break;
    case SectionType::Flange:
        result = 0;
        break;
    }
    return result;
}

std::string GetWeldingMethodStr(WeldingMethod value)
{
    std::string result;

    switch (value)
    {
    case WeldingMethod::RD:
        result = "РД";
        break;
    case WeldingMethod::RAD:
        result = "РАД";
        break;
    case WeldingMethod::MP:
        result = "МП";
        break;
    case WeldingMethod::MADP:
        result = "МАДП";
        break;
    case WeldingMethod::MF:
        result = "МФ";
        break;
    case WeldingMethod::MPS:
        result = "МПС";
        break;
    case WeldingMethod::AAD:
        result = "ААД";
        break;
    case WeldingMethod::AADP:
        result = "ААДП";
        break;
    case WeldingMethod::APG:
        result = "АПГ";
        break;
    case WeldingMethod::API:
        result = "АПИ";
        break;
    case WeldingMethod::AF:
        result = "АФ";
        break;
    case WeldingMethod::G:
        result = "Г";
        break;
    case WeldingMethod::P:
        result = "П";
        break;
    case WeldingMethod::ESH:
        result = "ЭШ";
        break;
    }
    return result;
}

std::string GetWeldingMethodName(WeldingMethod value)
{
    std::string result;

    switch (value)
    {
    case WeldingMethod::RD:
        result = "ручная дуговая покрытыми электродами (SMAW, MMA)";
        break;
    case WeldingMethod::RAD:
        result = "ручная аргонодуговая неплавящимся электродом (GTAW, TIG)";
        break;
    case WeldingMethod::MP:
        result = "механизированная плавящимся электродом в среде активных газов и смесях (GMAW, MAG)";
        break;
    case WeldingMethod::MADP:
        result = "механизированная аргонодуговая плавящимся электродом (GMAW, MIG)";
        break;
    case WeldingMethod::MF:
        result = "механизированная под флюсом (SAW)";
        break;
    case WeldingMethod::MPS:
        result = "механизированная самозащитной порошковой проволокой (FCAW-S)";
        break;
    case WeldingMethod::AAD:
        result = "автоматическая аргонодуговая неплавящимся электродом (GTAW, TIG)";
        break;
    case WeldingMethod::AADP:
        result = "автоматическая аргонодуговая плавящимся электродом (GMAW, MIG)";
        break;
    case WeldingMethod::APG:
        result = "автоматическая плавящимся электродом в среде защитных газов (GMAW)";
        break;
    case WeldingMethod::API:
        result = "автоматическая проволокой сплошного сечения в среде защитных газов (GMAW)";
        break;
    case WeldingMethod::AF:
        result = "автоматическая под флюсом (SAW)";
        break;
    case WeldingMethod::G:
        result = "газовая (OFW)";
        break;
    case WeldingMethod::P:
        result = "плазменная (PAW)";
        break;
    case WeldingMethod::ESH:
        result = "электрошлаковая (ESW)";
        break;
    }
    return result;
}

std::string GetWeldingMethodsStr(const std::vector<WeldingMethod> &values)
{
    std::string result;

    for (WeldingMethod value : values)
    {
        if (!result.empty())
            result += weldingMethodSeparator;
        result += GetWeldingMethodStr(value);
    }
    return result;
}

std::vector<WeldingMethod> ParseWeldingMethods(const std::string &value)
{
    std::vector<WeldingMethod> result;

    for (size_t begin = 0; begin < value.size();)
    {
        size_t end = value.find(weldingMethodSeparator, begin);
        if (end == std::string::npos)
            end = value.size();

        if (end > begin)
        {
            // в базе может лежать "РД + АПИ" - обозначение берём без окружающих пробелов
            const size_t tokenBegin = value.find_first_not_of(' ', begin);
            if (tokenBegin != std::string::npos && tokenBegin < end)
            {
                const size_t tokenEnd = value.find_last_not_of(' ', end - 1);
                const std::string token = value.substr(tokenBegin, tokenEnd - tokenBegin + 1);

                for (int i = 0; i < static_cast<int>(WeldingMethod::Count); ++i)
                {
                    auto item = static_cast<WeldingMethod>(i);
                    if (GetWeldingMethodStr(item) == token)
                    {
                        result.push_back(item);
                        break;
                    }
                }
            }
        }
        begin = end + 1;
    }

    return result;
}
