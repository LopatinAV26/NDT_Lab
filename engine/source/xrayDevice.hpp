#pragma once

#include <string>
#include <vector>

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

private:
};