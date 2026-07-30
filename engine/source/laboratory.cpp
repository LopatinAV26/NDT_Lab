#include "laboratory.hpp"

Protocol::Protocol()
{
	// metalOptDenMax = static_cast<float>(std::log10(negatoscopeBrightness) - 2.0f);
	// opticalDensityTitle = std::format("Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
	// weldOptDenMin, metalOptDenMax, negatoscopeBrightness);
}

namespace NDT
{
	std::string GetCurrentDateString()
	{
		auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
		return std::format("{:%d.%m.%Y}", today);
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
		return diam * 3.14159f;
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

	float GetMetalDensity(int negBright)
	{
		// opticalDensityTitle = "";
		return static_cast<float>(std::log10(negBright) - 2.0f);
	}
}
