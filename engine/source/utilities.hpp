#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include "imgui.h"

namespace NDT
{
	/// @brief Знак "(?)" с всплывающей подсказкой при наведении
	/// @param desc Текст подсказки
	void HelpMarker(const char *desc);

	/// @brief Получить текущую локальную дату в формате ISO 8601 (YYYY-MM-DD) - для хранения в БД
	/// @return std::string в формате ISO 8601 (YYYY-MM-DD)
	std::string GetCurrentIsoDate();

	/// @brief Разобрать дату в формате ISO 8601 (YYYY-MM-DD);
	/// @brief при ошибке разбора возвращает 1900-01-01
	/// @return std::tm
	tm ParseIsoDateTm(const std::string &isoDate);

	/// @brief Преобразовать дату tm в формат ISO 8601 (YYYY-MM-DD) - для хранения в БД
	/// @return std::string в формате ISO 8601 (YYYY-MM-DD)
	std::string FormatIsoDateTm(const tm &dateTm);

	/// @brief Преобразовать ISO-дату в формат ДД.ММ.ГГГГ - для отображения в UI
	/// @return std::string в формате ДД.ММ.ГГГГ
	std::string FormatDateForDisplay(const std::string &isoDate);

	std::chrono::year_month_day GetTerm(const std::chrono::year_month_day &date1,
										const std::chrono::year_month_day &date2 = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now()));

	/// @brief Разобрать дату в формате ISO 8601 (YYYY-MM-DD) в std::chrono::year_month_day
	std::chrono::year_month_day ParseIsoDate(const std::string &isoDate);

	/// @brief Отформатировать разницу дат (результат GetTerm) в строку вида "5 лет 3 месяца 12 дней"
	std::string FormatTerm(const std::chrono::year_month_day &term);

	/// @brief Стаж на сегодняшний день, вычисляется как разница между текущей датой и датой трудоустройства
	std::string GetExperience(const std::string &isoDate);

	/// @brief Вычисление количества участков по 300 мм
	/// @param diam диаметр свариваемых труб
	/// @return Количество участков
	std::vector<std::string> CalculateNumString(int diam, int range = 300);

	/// @brief Максимально допустимая плотность снимка, в зависимости от яркости негатоскопа
	/// @param negBright паспортная яркость негатоскопа
	/// @return Плотность снимка в е.о.п.
	float GetMetalDensity(int negBright);

	/// @brief Сгенерировать UUID v7 (первые 48 бит - unix-время в мс, остальное - случайность)
	std::string GenerateUuidV7();

	/// @brief Сбросить fileData во временный файл с именем fileName и открыть его
	/// ассоциированным приложением ОС (через ImGui::GetPlatformIO().Platform_OpenInShellFn -
	/// ShellExecuteW на Windows, system("open ...")/xdg-open на macOS/Linux)
	void OpenFileFromBytes(const std::string &fileName, const std::vector<std::uint8_t> &fileData);

	/// @brief Построить std::filesystem::path из UTF-8 строки в обход текущей кодовой страницы ОС
	/// (обычный конструктор path(std::string)/path::string() на Windows трактует байты через ANSI-
	/// кодовую страницу процесса - кириллица в имени файла ломается, если она не совпадает с CP1251)
	std::filesystem::path PathFromUtf8(const std::string &utf8);

	/// @brief Получить путь в виде UTF-8 std::string в обход текущей кодовой страницы ОС
	std::string PathToUtf8(const std::filesystem::path &path);

	/// @brief Привести UTF-8 строку к нижнему регистру (ASCII + кириллица) - для поиска без учёта регистра
	std::string ToLowerUtf8(const std::string &s);

	/// @brief Рисует однострочный текст ячейки таблицы; если он не влезает по ширине столбца - при наведении
	/// показывает тултип с полным содержимым (по аналогии с тем, как ImGui сам подсказывает обрезанные заголовки таблицы)
	void TextWithTooltipIfTruncated(const std::string &text);
}