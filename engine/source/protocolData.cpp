#include "protocolData.hpp"

ProtocolData::ProtocolData()
{
	metalOptDenMax = static_cast<float>(std::log10(negatoscopeBrightness) - 2.0f);
	opticalDensityTitle = std::format("Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
		weldOptDenMin, metalOptDenMax, negatoscopeBrightness);
}

float ProtocolData::GetPerimeter(int diam)
{
	return diam * 3.14159f;
}

float ProtocolData::GetMetalDensity(int negBright)
{
	opticalDensityTitle = "";
	return static_cast<float>(std::log10(negBright) - 2.0f);
}

int ProtocolData::CalculateString(int diam)
{
	constexpr float range = 300.f;
	float perimeter = GetPerimeter(diam);
	constexpr float eps = 2.0f; ///< необходимо подобрать экспериментально чтобы не было лишнего или недостающего участка
	return static_cast<int>(std::ceil((perimeter - eps) / range));
}
