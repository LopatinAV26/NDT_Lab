#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace NDT
{
	struct Curve
	{
		float voltage{ 1.f };
		std::string label;
		std::vector<float> xData;
		std::vector<float> yData;
	};

	struct XrayDevice
	{
		std::string name;
		std::string information;
		float electricPower{ 1.f };
		float voltageMaximum{ 200.f };
		float currentMinimum{ 0.1f };
		float currentMaximum{ 1.f };
		float focusDistanceDefault{ 700.f };
		float focalSpotSize{ 1.f };
		float exposureMultiplier{ 1.f };
		//float steelThicknessMax{ 1.f };
		bool currentAdjustment{ true };
		std::vector<Curve> curveVector;
	};

	std::vector<XrayDevice> LoadDevices(const std::filesystem::path& pathToDevices);
}