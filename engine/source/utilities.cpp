#include "utilities.hpp"

#include <cstdio>
#include <cmath>

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
}

namespace NDT
{
	void HelpMarker(const char *desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	std::string GetCurrentDateString()
	{
		auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
		return std::format("{:%d.%m.%Y}", today);
	}

	tm ParseDateString(const std::string &date)
	{
		int year = 1900, month = 1, day = 1;
		std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day);

		tm result{};
		result.tm_isdst = -1;
		result.tm_mday = day;
		result.tm_mon = month - 1;
		result.tm_year = year - 1900;
		return result;
	}

	std::string FormatDateString(const tm &date)
	{
		return std::format("{:04d}-{:02d}-{:02d}", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday);
	}

	std::string FormatDateForDisplay(const std::string &isoDate)
	{
		tm date = ParseDateString(isoDate);
		return std::format("{:02d}.{:02d}.{:04d}", date.tm_mday, date.tm_mon + 1, date.tm_year + 1900);
	}

	std::vector<std::string> CalculateNumString(int diam, int range)
	{
		float perimeter = GetPerimeter(diam);
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

	float GetPerimeter(int diam)
	{
		return diam * M_PI;
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

		int y = 1900, m = 1, d = 1;
		std::sscanf(isoDate.c_str(), "%d-%d-%d", &y, &m, &d);

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

	std::string GetEmployeeExperience(const std::string &employeementDateIso)
	{
		return FormatTerm(GetTerm(ParseIsoDate(employeementDateIso)));
	}

	float GetMetalDensity(int negBright)
	{
		// opticalDensityTitle = "";
		return static_cast<float>(std::log10(negBright) - 2.0f);
	}
	std::string GenerateUuidV4()
	{
		static std::random_device rd;
		static std::mt19937_64 gen(rd());
		static std::uniform_int_distribution<uint64_t> dist;

		uint64_t a = dist(gen);
		uint64_t b = dist(gen);

		// версия 4 ("случайный" UUID) - биты 12-15 второй половины `a` = 0100
		a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
		// вариант RFC 4122 - два старших бита `b` = "10"
		b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

		return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
						   static_cast<uint32_t>(a >> 32),
						   static_cast<uint16_t>(a >> 16),
						   static_cast<uint16_t>(a),
						   static_cast<uint16_t>(b >> 48),
						   b & 0xFFFFFFFFFFFFULL);
	}
}
