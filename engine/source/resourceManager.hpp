#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace NDT
{
    struct Curve
    {
        std::string label;
        int voltage{0};
        std::vector<float> xData;
        std::vector<float> yData;
    };

    struct XrayDevice
    {
        std::string name;
        std::string information;
        int electricPower{0};
        int voltageMaximum{200};
        bool currentAdjustment{true};
        float currentMinimum{0.1f};
        float currentMaximum{1.f};
        int focusDistanceDefault{700};
        float focalSpotSize{1.0f};
        float exposureMultiplier{1.f};
        float steelThicknessMax{1.f};

        std::vector<Curve> curveVector;
    };

    std::vector<XrayDevice> LoadDevices(const std::filesystem::path &pathToDevices);
}