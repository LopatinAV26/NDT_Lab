#pragma once

#include <vector>
#include <filesystem>
#include <toml++/toml.hpp>

struct XrayDevice
{
    std::string_view name;
    std::string_view information;
    float electricPower{0.f};
    float voltageMaximum{200.f};
    bool currentAdjustment{true};
    float currentMinimum{0.1f};
    float currentMaximum{1.f};
    int focusDistanceDefault{700};
    float focalSpotSize{1.0f};
    float exposureMultiplier{1.f};
};

class ResourceManager
{
public:
    ResourceManager();

    std::vector<XrayDevice> LoadDevices();

private:
    const std::filesystem::path pathToDevices{"resources/xray_devices.toml"};
    toml::table tbl;
};