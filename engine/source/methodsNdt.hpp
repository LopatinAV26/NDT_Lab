#pragma once

#include <cstdint>
#include <string>

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
    IV
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

std::string GetMethodAbbreviation(Method value);
std::string GetCategoryStr(Category value);
std::string GetWeldJointTypeStr(WeldJointType value);
