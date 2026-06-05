#include "resourceManager.hpp"
#include <SDL3/SDL.h>

ResourceManager::ResourceManager()
{
}

std::vector<XrayDevice> ResourceManager::LoadDevices()
{
    std::vector<XrayDevice> devices;

    try
    {
        tbl = toml::parse_file(pathToDevices.string());
        SDL_Log("Devices Config file '%s' parsed successfully.\n", pathToDevices.c_str());
    }
    catch (const toml::parse_error &err)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to parse config file '%s': %s\n",
                     pathToDevices.c_str(), err.what());
        return devices;
    }

    auto *deviceArray = tbl["devices"].as_array();

    devices.reserve(deviceArray->size());

    for (const auto &node : *deviceArray)
    {
        const auto *deviceTable = node.as_table();
        if (!deviceTable)
            continue;

        XrayDevice device;
        if (auto *value = deviceTable->get("name"))
            device.name = value->value_or("");

        if (auto *value = deviceTable->get("information"))
            device.information = value->value_or("");

        if (auto *value = deviceTable->get("electricPower"))
            device.electricPower = value->value_or(0.f);

        if (auto *value = deviceTable->get("voltageMaximum"))
            device.voltageMaximum = value->value_or(0.f);

        if (auto *value = deviceTable->get("currentAdjustment"))
            device.currentAdjustment = value->value_or(false);

        if (auto *value = deviceTable->get("currentMinimum"))
            device.currentMinimum = value->value_or(0.f);

        if (auto *value = deviceTable->get("currentMaximum"))
            device.currentMaximum = value->value_or(0.f);

        if (auto *value = deviceTable->get("focusDistanceDefault"))
            device.focusDistanceDefault = value->value_or(0.f);

        if (auto *value = deviceTable->get("focalSpotSize"))
            device.focalSpotSize = value->value_or(0.f);

        if (auto *value = deviceTable->get("exposureMultiplier"))
            device.exposureMultiplier = value->value_or(1.f);

        devices.push_back(device);
    }
    return devices;
}
