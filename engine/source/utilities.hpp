#pragma once

#include <string>
#include <format>
#include <vector>
#include <chrono>
#include <optional>
#include <random>
#include <cstdint>

namespace NDT
{
	/// @brief Получить текущую локальную дату
	std::string GetCurrentDateString();

	std::chrono::year_month_day GetTerm(const std::chrono::year_month_day &date1,
										const std::chrono::year_month_day &date2 = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));

	/// @brief Вычисление количества участков по 300 мм
	/// @param diam диаметр свариваемых труб
	/// @return Количество участков
	std::vector<std::string> CalculateNumString(int diam, int range = 300);

	float GetPerimeter(int diam);

	/// @brief Максимально допустимая плотность снимка, в зависимости от яркости негатоскопа
	/// @param negBright паспортная яркость негатоскопа
	/// @return Плотность снимка в е.о.п.
	float GetMetalDensity(int negBright);

	std::string GenerateUuidV4();
}