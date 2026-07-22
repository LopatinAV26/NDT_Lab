#include "protocol.hpp"

Protocol::Protocol()
{
	metalOptDenMax = static_cast<float>(std::log10(negatoscopeBrightness) - 2.0f);
	opticalDensityTitle = std::format("Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
									  weldOptDenMin, metalOptDenMax, negatoscopeBrightness);
}

float Protocol::GetPerimeter(int diam) const
{
	return diam * 3.14159f;
}

float Protocol::GetMetalDensity(int negBright)
{
	opticalDensityTitle = "";
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

Defect Protocol::CreateDefectRGC(DefRGCData input)
{
	Defect defect{
		.width = {input.width},
		.height = {input.height},
		.coord = {input.coord},
		.length = {input.length}};

	// Ас 25.0 – 2.0 × 1.0 ≤	пример записи дефекта
	// A   B   C  D  E  F  G
	std::string A = input.name.at(input.nameIndex);
	std::string B;
	// std::string C = input.spacer.at(input.spacerIndex1);
	std::string C = "-";
	std::string D = std::format("{:.1f}", defect.width);
	// std::string E = input.spacer.at(input.spacerIndex2);
	std::string E = "×";
	std::string F = std::format("{:.1f}", defect.height);
	std::string G = input.end.at(input.endIndex);
	// std::string coord = std::format("{:d}", input.coord);

	if (A == "Ac" || A == "Ab")
		B = std::format("{:.1f}", defect.length);
	else
	{
		B = "";
		C = "";
	}

	defect.coordStr = std::format("({:d}) ", defect.coord);
	defect.record = A + " " + B + C + D + E + F + G;

	// Аа3,0≤

	// Ас25–2×1,0≤

	// E100

	// Fe>

	return defect;
}
