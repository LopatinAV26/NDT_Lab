#pragma once

#include <string>
#include <vector>
#include <toml++/toml.hpp>

struct Point
{
    int voltage;
    float x;
    float y;
};

struct DeviceData
{
    std::string deviceName;
    std::string information;
    float electric_power = 0.f;
    float kVmaximum = 0.f;
    float mAminimum = 0.f;
    float mAmaximum = 0.f;
    int focusDistance = 0;
};

class XrayDevice
{
public:
    void SetDeviceData()
    {
        data = toml::parse_file("resources/xray_devices.toml");
    };

private:

    toml::table data;
    std::vector<Point> points;
};