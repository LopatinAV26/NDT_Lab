#pragma once

#include <cstdint>
#include <string>
#include <vector>

/// @brief метод НК
enum class Method : uint8_t
{
    VT,   /// визуальный и измерительный контроль
    RT,   /// радиографический контроль
    DRT,  /// цифровой радиографический контроль
    UT,   /// ультразвуковой контроль
    PT,   /// капиллярный контроль
    LT,   /// течеискание
    MT,   /// магнитопорошковый контроль
    DT,   /// Delamination - расслоение
    ECT,  /// Eddy Current Testing - вихретоковый контроль
    UTM,  /// Ultrasonic Thickness Measurement - ультразвуковое измерение толщины
    Count ///< количество элементов
};

/// @brief категория трубопровода
enum class Category : uint8_t
{
    H, /// высшая
    I,
    II,
    III,
    IV,
    Count ///< количество элементов
};

/// @brief тип сварного соединения
enum class WeldJointType : uint8_t
{
    Butt,   /// стыковое
    Corner, /// угловое
    Tjoint, /// тавровое
    Lap,    /// нахлёсточное
    Count
};

/// @brief тип элемента, сваренного в стыке: секция трубы по количеству продольных швов либо деталь
enum class SectionType : uint8_t
{
    SingleSeam, /// одношовная
    DoubleSeam, /// двухшовная
    Seamless,   /// бесшовная
    Transition, /// переход с одной толщины стенки на другую
    Flange,     /// фланец
    Count
};

/// @brief способ сварки, обозначения по РД 03-495-02
enum class WeldingMethod : uint8_t
{
    RD,   /// ручная дуговая покрытыми электродами
    RAD,  /// ручная аргонодуговая неплавящимся электродом
    MP,   /// механизированная плавящимся электродом в среде активных газов и смесях
    MADP, /// механизированная аргонодуговая плавящимся электродом
    MF,   /// механизированная под флюсом
    MPS,  /// механизированная самозащитной порошковой проволокой
    AAD,  /// автоматическая аргонодуговая неплавящимся электродом
    AADP, /// автоматическая аргонодуговая плавящимся электродом
    APG,  /// автоматическая плавящимся электродом в среде защитных газов
    API,  /// автоматическая проволокой сплошного сечения в среде защитных газов (СТО Газпром)
    AF,   /// автоматическая под флюсом
    G,    /// газовая
    P,    /// плазменная
    ESH,  /// электрошлаковая
    Count
};

std::string GetMethodAbbreviation(Method value);
std::string GetCategoryStr(Category value);
std::string GetWeldJointTypeStr(WeldJointType value);

/// @brief Обратное преобразование к GetWeldJointTypeStr - для разбора значения при загрузке из БД
/// (хранить нужно именно строку, а не число перечисления: порядок объявления не должен
/// становиться частью формата хранения)
/// @return при неизвестной строке возвращает WeldJointType::Butt
WeldJointType ParseWeldJointType(const std::string &value);

std::string GetSectionTypeStr(SectionType value);

/// @brief Обратное преобразование к GetSectionTypeStr - для разбора значения при загрузке из БД
/// @return при неизвестной строке возвращает SectionType::SingleSeam
SectionType ParseSectionType(const std::string &value);

/// @brief Количество продольных швов у элемента - столько координат и нужно вводить в заключении
/// @return 0 для бесшовной секции и фланца, 1 для одношовной и перехода, 2 для двухшовной
int GetSeamCount(SectionType value);

std::string GetWeldingMethodStr(WeldingMethod value);

/// @brief Полное наименование способа сварки - для подсказки к обозначению
std::string GetWeldingMethodName(WeldingMethod value);

/// @brief Собрать обозначения способов сварки в строку для бланка: "РД+АПИ"
/// (стык варят комбинацией способов - корень одним, заполнение другим)
std::string GetWeldingMethodsStr(const std::vector<WeldingMethod> &values);

/// @brief Разобрать строку вида "РД+АПИ" - для загрузки из БД
/// @return неизвестные обозначения пропускаются
std::vector<WeldingMethod> ParseWeldingMethods(const std::string &value);
