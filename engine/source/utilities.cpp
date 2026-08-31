#include "utilities.hpp"

#include <charconv>
#include <format>
#include <random>
#include <cmath>
#include <fstream>
#include <filesystem>

// RenderTextEllipsis - внутренняя функция ImGui (используется самим ImGui для заголовков таблиц),
// авторы просят не использовать её вне imgui.cpp - сигнатура может измениться при обновлении ImGui
#include "imgui_internal.h"

namespace
{
	/// @brief Согласование числительного с существительным в русском языке (1 год, 2 года, 5 лет)
	std::string RussianPlural(int n, const std::string &one, const std::string &few, const std::string &many)
	{
		int mod100 = n % 100;
		int mod10 = n % 10;

		if (mod100 >= 11 && mod100 <= 14)
			return many;
		if (mod10 == 1)
			return one;
		if (mod10 >= 2 && mod10 <= 4)
			return few;

		return many;
	}

	/// @brief Разобрать год/месяц/день из строки ISO 8601 (YYYY-MM-DD, фиксированная ширина);
	/// при слишком короткой/пустой строке оставляет значения по умолчанию (2000-01-01)
	void ParseIsoDateParts(const std::string &date, int &year, int &month, int &day)
	{
		year = 2000;
		month = 1;
		day = 1;

		if (date.size() < 10)
			return;

		std::from_chars(date.data(), date.data() + 4, year);
		std::from_chars(date.data() + 5, date.data() + 7, month);
		std::from_chars(date.data() + 8, date.data() + 10, day);
	}
}

namespace NDT
{
	void HelpMarker(const char *desc)
	{
		ImGui::Text("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	std::string GetCurrentIsoDate()
	{
		auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
		return std::format("{:%Y-%m-%d}", today);
	}

	tm ParseIsoDateTm(const std::string &isoDate)
	{
		int year, month, day;
		ParseIsoDateParts(isoDate, year, month, day);

		tm result{};
		result.tm_isdst = -1;
		result.tm_mday = day;
		result.tm_mon = month - 1;
		result.tm_year = year - 1900;
		return result;
	}

	std::string FormatIsoDateTm(const tm &dateTm)
	{
		return std::format("{:04d}-{:02d}-{:02d}", dateTm.tm_year + 1900, dateTm.tm_mon + 1, dateTm.tm_mday);
	}

	std::string FormatDateForDisplay(const std::string &isoDate)
	{
		tm date = ParseIsoDateTm(isoDate);
		return std::format("{:02d}.{:02d}.{:04d}", date.tm_mday, date.tm_mon + 1, date.tm_year + 1900);
	}

	std::vector<std::string> CalculateNumString(int diam, int range)
	{
		float perimeter = diam * 3.14159265358979323846f;
		constexpr float eps = 2.0f; ///< необходимо подобрать экспериментально чтобы не было лишнего или недостающего участка
		int result = static_cast<int>(std::ceil((perimeter - eps) / range));
		std::vector<std::string> rangesStringList;
		rangesStringList.reserve(result);
		int coord = 0;

		for (int i = 0; i < result; ++i)
		{
			if (coord + range > perimeter - eps)
				rangesStringList.push_back(std::format("{:d}-{:d}", coord, 0));
			else
				rangesStringList.push_back(std::format("{:d}-{:d}", coord, coord + range));

			coord += range;
		}

		return rangesStringList;
	}

	std::chrono::year_month_day GetTerm(const std::chrono::year_month_day &date1, const std::chrono::year_month_day &date2)
	{
		using namespace std::chrono;

		int y = static_cast<int>(date2.year()) - static_cast<int>(date1.year());
		int m = static_cast<int>(static_cast<unsigned>(date2.month())) - static_cast<int>(static_cast<unsigned>(date1.month()));
		int d = static_cast<int>(static_cast<unsigned>(date2.day())) - static_cast<int>(static_cast<unsigned>(date1.day()));

		if (d < 0)
		{
			--m;
			auto prevMonth = (date2.year() / date2.month()) - months{1};
			d += static_cast<int>(static_cast<unsigned>((prevMonth.year() / prevMonth.month() / last).day()));
		}

		if (m < 0)
		{
			--y;
			m += 12;
		}

		return year_month_day{year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}};
	}

	std::chrono::year_month_day ParseIsoDate(const std::string &isoDate)
	{
		using namespace std::chrono;

		int y, m, d;
		ParseIsoDateParts(isoDate, y, m, d);

		return year_month_day{year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}};
	}

	std::string FormatTerm(const std::chrono::year_month_day &term)
	{
		int years = static_cast<int>(term.year());
		int months = static_cast<int>(static_cast<unsigned>(term.month()));
		int days = static_cast<int>(static_cast<unsigned>(term.day()));

		std::vector<std::string> parts;
		if (years > 0)
			parts.push_back(std::format("{:d} {:s}", years, RussianPlural(years, "год", "года", "лет")));
		if (months > 0)
			parts.push_back(std::format("{:d} {:s}", months, RussianPlural(months, "месяц", "месяца", "месяцев")));
		if (days > 0 || parts.empty()) // "0 дней" - если стаж меньше суток
			parts.push_back(std::format("{:d} {:s}", days, RussianPlural(days, "день", "дня", "дней")));

		std::string result;
		for (size_t i = 0; i < parts.size(); ++i)
		{
			if (i > 0)
				result += " ";
			result += parts[i];
		}

		return result;
	}

	std::string GetExperience(const std::string &isoDate)
	{
		return FormatTerm(GetTerm(ParseIsoDate(isoDate)));
	}

	float GetMetalDensity(int negBright)
	{
		// opticalDensityTitle = "";
		return static_cast<float>(std::log10(negBright) - 2.0f);
	}

	std::string GenerateUuidV7()
	{
		static std::random_device rd;
		static std::mt19937_64 gen(rd());
		static std::uniform_int_distribution<uint64_t> dist;

		uint64_t unixMs = static_cast<uint64_t>(
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

		uint64_t a = (unixMs << 16) | (dist(gen) & 0xFFFFULL);
		uint64_t b = dist(gen);

		// версия 7 (время + случайность) - биты 12-15 второй половины `a` = 0111
		a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000007000ULL;
		// вариант RFC 4122 - два старших бита `b` = "10"
		b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

		return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
						   static_cast<uint32_t>(a >> 32),
						   static_cast<uint16_t>(a >> 16),
						   static_cast<uint16_t>(a),
						   static_cast<uint16_t>(b >> 48),
						   b & 0xFFFFFFFFFFFFULL);
	}

	std::filesystem::path PathFromUtf8(const std::string &utf8)
	{
		const std::u8string utf8AsChar8(reinterpret_cast<const char8_t *>(utf8.data()), utf8.size());
		return std::filesystem::path(utf8AsChar8);
	}

	std::string PathToUtf8(const std::filesystem::path &path)
	{
		const std::u8string pathUtf8 = path.u8string();
		return std::string(reinterpret_cast<const char *>(pathUtf8.data()), pathUtf8.size());
	}

	void OpenFileFromBytes(const std::string &fileName, const std::vector<std::uint8_t> &fileData)
	{
		std::filesystem::path tempPath = std::filesystem::temp_directory_path() / PathFromUtf8(fileName);

		std::ofstream tempFile(tempPath, std::ios::binary | std::ios::trunc);
		tempFile.write(reinterpret_cast<const char *>(fileData.data()), static_cast<std::streamsize>(fileData.size()));
		tempFile.close();

		ImGui::GetPlatformIO().Platform_OpenInShellFn(ImGui::GetCurrentContext(), PathToUtf8(tempPath).c_str());
	}

	std::string ToLowerUtf8(const std::string &s)
	{
		std::string out;
		out.reserve(s.size());
		for (size_t i = 0; i < s.size();)
		{
			unsigned char c0 = static_cast<unsigned char>(s[i]);
			if (c0 < 0x80)
			{
				out += static_cast<char>(std::tolower(c0));
				++i;
			}
			else if (c0 == 0xD0 && i + 1 < s.size())
			{
				unsigned char c1 = static_cast<unsigned char>(s[i + 1]);
				if (c1 == 0x81) // Ё -> ё
				{
					out += static_cast<char>(0xD1);
					out += static_cast<char>(0x91);
				}
				else if (c1 >= 0x90 && c1 <= 0x9F) // А-П -> а-п
				{
					out += static_cast<char>(0xD0);
					out += static_cast<char>(c1 + 0x20);
				}
				else if (c1 >= 0xA0 && c1 <= 0xAF) // Р-Я -> р-я
				{
					out += static_cast<char>(0xD1);
					out += static_cast<char>(c1 - 0x20);
				}
				else
				{
					out += static_cast<char>(c0);
					out += static_cast<char>(c1);
				}
				i += 2;
			}
			else if (i + 1 < s.size())
			{
				out += static_cast<char>(c0);
				out += s[i + 1];
				i += 2;
			}
			else
			{
				out += static_cast<char>(c0);
				++i;
			}
		}
		return out;
	}

	void TextWithTooltipIfTruncated(const std::string &text)
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		float available = ImGui::GetContentRegionAvail().x;
		float lineHeight = ImGui::GetTextLineHeight();
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

		/// RenderTextEllipsis сама рисует "..." при обрезке - ровно то же поведение, что у заголовков таблиц ImGui
		ImGui::RenderTextEllipsis(ImGui::GetWindowDrawList(), pos, ImVec2(pos.x + available, pos.y + lineHeight),
								  pos.x + available, text.c_str(), text.c_str() + text.size(), &textSize);
		ImGui::Dummy(ImVec2(available, lineHeight)); /// RenderTextEllipsis не создаёт item сама - вручную продвигаем курсор макета

		if (textSize.x > available && ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", text.c_str());
	}

}
