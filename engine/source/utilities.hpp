#pragma once

#include <string>
#include <format>
#include <vector>
#include <chrono>
#include <optional>
#include <random>
#include <cstdint>
#include <ctime>
#include "imgui.h"

namespace NDT
{
	/// @brief Знак "(?)" с всплывающей подсказкой при наведении
	/// @param desc Текст подсказки
	void HelpMarker(const char *desc);

	/// @brief Получить текущую локальную дату в формате ISO 8601 (YYYY-MM-DD) - для хранения
	std::string GetCurrentIsoDate();

	/// @brief Разобрать дату в формате ISO 8601 (YYYY-MM-DD);
	/// @brief при ошибке разбора возвращает 1900-01-01
	/// @return std::tm
	tm ParseIsoDateTm(const std::string &date);

	/// @brief Преобразовать дату tm в формат ISO 8601 (YYYY-MM-DD) - для хранения в БД
	/// @return std::string в формате ISO 8601 (YYYY-MM-DD)
	std::string FormatIsoDateTm(const tm &date);

	/// @brief Преобразовать ISO-дату в формат ДД.ММ.ГГГГ - для отображения в UI
	/// @return std::string в формате ДД.ММ.ГГГГ
	std::string FormatDateForDisplay(const std::string &isoDate);

	std::chrono::year_month_day GetTerm(const std::chrono::year_month_day &date1,
										const std::chrono::year_month_day &date2 = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));

	/// @brief Разобрать дату в формате ISO 8601 (YYYY-MM-DD) в std::chrono::year_month_day
	std::chrono::year_month_day ParseIsoDate(const std::string &isoDate);

	/// @brief Отформатировать разницу дат (результат GetTerm) в строку вида "5 лет 3 месяца 12 дней"
	std::string FormatTerm(const std::chrono::year_month_day &term);

	/// @brief Стаж сотрудника на сегодняшний день, вычисляется от даты трудоустройства (ISO 8601) - не хранится в БД
	std::string GetEmployeeExperience(const std::string &employeementDateIso);

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