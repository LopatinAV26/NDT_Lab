#include "protocol.hpp"

#include <chrono>

Protocol::Protocol()
{
	// metalOptDenMax = static_cast<float>(std::log10(negatoscopeBrightness) - 2.0f);
	// opticalDensityTitle = std::format("Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
	// weldOptDenMin, metalOptDenMax, negatoscopeBrightness);
}

float Protocol::GetPerimeter(int diam) const
{
	return diam * 3.14159f;
}

float Protocol::GetMetalDensity(int negBright)
{
	// opticalDensityTitle = "";
	return static_cast<float>(std::log10(negBright) - 2.0f);
}

/* int ProtocolData::CalculateString(int diam)
{
	constexpr float range = 300.f;
	float perimeter = GetPerimeter(diam);
	constexpr float eps = 2.0f; ///< необходимо подобрать экспериментально чтобы не было лишнего или недостающего участка
	return static_cast<int>(std::ceil((perimeter - eps) / range));
} */

int Protocol::CalculateNumString(int numDefects)
{
	return numDefects;
	////дополнить логикой объединения однотипных дефектов
}

void Protocol::ConstructDefectRGCString(DefRGC &input)
{
	// Ас 25.0 – 2.0 × 1.0 ≤	пример записи дефекта
	// A   B   C  D  E  F  G
	std::string A = input.name.at(input.nameIndex);
	std::string B;
	std::string C = "-";
	std::string D = std::format("{:.1f}", input.width);
	std::string E = "×";
	std::string F = std::format("{:.1f}", input.height);
	std::string G = input.end.at(input.endIndex);

	if (A == "Ac" || A == "Ab")
		B = std::format("{:.1f}", input.length);
	else
	{
		B = "";
		C = "";
	}

	input.coordStr = std::format("({:d}) ", input.coord);
	input.record = A + " " + B + C + D + E + F + G;
}

namespace NDT
{
	std::string NDT::GetCurrentDateString()
	{
		auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
		return std::format("{:%d.%m.%Y}", today);
	}
}
