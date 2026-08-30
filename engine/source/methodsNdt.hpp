#pragma once

#include <cstdint>
#include <string>

enum class Method : uint8_t
{
    VT,  /// визуальный и измерительный контроль
    RT,  /// радиографический контроль
    DRT, /// цифровой радиографический контроль
    UT,  /// ультразвуковой контроль
    PT,  /// капиллярный контроль
    LT,  /// течеискание
    MT,  /// магнитопорошковый контроль
    DT,  /// Delamination - расслоение
    ECT, /// Eddy Current Testing - вихретоковый контроль
    UTM,  /// Ultrasonic Thickness Measurement - ультразвуковое измерение толщины
    Count ///< количество элементов
};

std::string GetMethodAbbreviation(Method value);
