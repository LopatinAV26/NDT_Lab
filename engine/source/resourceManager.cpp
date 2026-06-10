#include "resourceManager.hpp"
#include <SDL3/SDL.h>
#include <toml++/toml.hpp>

std::vector<NDT::XrayDevice> NDT::LoadDevices(const std::filesystem::path &pathToDevices)
{
    std::vector<XrayDevice> devices;
    toml::table tbl;

    try
    {
        tbl = toml::parse_file(pathToDevices.string());
        SDL_Log("Devices Config file '%s' parsed successfully.\n", pathToDevices.string().c_str());
    }
    catch (const toml::parse_error &err)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to parse config file '%s': %s\n",
                     pathToDevices.string().c_str(), err.what());
        return devices;
    }

    auto *deviceArray = tbl["devices"].as_array();
    if (!deviceArray)
        return devices;

    devices.reserve(deviceArray->size());

    for (const auto &deviceNode : *deviceArray)
    {
        const auto *deviceTable = deviceNode.as_table();
        if (!deviceTable)
            continue;

        XrayDevice device;
        if (auto *value = deviceTable->get("name"))
            device.name = value->value_or("");

        if (auto *value = deviceTable->get("information"))
            device.information = value->value_or("");

        if (auto *value = deviceTable->get("electricPower"))
            device.electricPower = value->value_or(0);

        if (auto *value = deviceTable->get("voltageMaximum"))
            device.voltageMaximum = value->value_or(0);

        if (auto *value = deviceTable->get("currentAdjustment"))
            device.currentAdjustment = value->value_or(false);

        if (auto *value = deviceTable->get("currentMinimum"))
            device.currentMinimum = value->value_or(0.f);

        if (auto *value = deviceTable->get("currentMaximum"))
            device.currentMaximum = value->value_or(0.f);

        if (auto *value = deviceTable->get("focusDistanceDefault"))
            device.focusDistanceDefault = value->value_or(0);

        if (auto *value = deviceTable->get("focalSpotSize"))
            device.focalSpotSize = value->value_or(0.f);

        if (auto *value = deviceTable->get("exposureMultiplier"))
            device.exposureMultiplier = value->value_or(1.f);

        if (auto *value = deviceTable->get("steelThicknessMax"))
            device.steelThicknessMax = value->value_or(1.f);

        auto *curvesArray = deviceTable->get_as<toml::array>("curves");
        if (!curvesArray)
            return devices;

        for (const auto &curveNode : *curvesArray)
        {
            const auto *curvesTable = curveNode.as_table();
            if (!curvesTable)
                continue;

            Curve curve;
            if (auto *value = curvesTable->get("name"))
                curve.label = {value->value_or("")};

            if (auto *value = curvesTable->get("voltage"))
                curve.voltage = {value->value_or(0)};

            if (auto *pointsArray = curvesTable->get_as<toml::array>("points"))
            {
                curve.xData.reserve(pointsArray->size());
                curve.yData.reserve(pointsArray->size());

                for (const auto &pointsNode : *pointsArray)
                {
                    const auto *point = pointsNode.as_array();
                    if (!point || point->size() != 2)
                        continue;

                    auto x = point->at(0).value<float>();
                    auto y = point->at(1).value<float>();

                    if (x && y)
                    {
                        curve.xData.push_back(x.value());
                        curve.yData.push_back(y.value());
                    }
                }
            }
            device.curveVector.push_back(curve);
        }
        devices.push_back(device);
    }
    return devices;
}
