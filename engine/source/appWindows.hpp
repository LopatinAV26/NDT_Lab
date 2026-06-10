#pragma once

#include <vector>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "implot.h"

#include "resourceManager.hpp"
#include "applicationData.hpp"

class SettingsWindow
{
public:
    explicit SettingsWindow(ApplicationData &coreAppData);
    void Show(bool &isOpen);

private:
    void SetGuiStyle();

    ApplicationData &appData;
    float fpsUpdateTimer{0.f};
    float currentFrametime{0.f};
    float framerate{0.f};
};

class NomogramWindow
{
public:
    explicit NomogramWindow(ApplicationData &coreAppData);
    void Show(bool &isOpen);

private:
    ApplicationData &appData;
    std::vector<NDT::XrayDevice> devices;
    std::vector<NDT::XrayDevice> calculatedDevices;
    int deviceIndex{0};
    int focusDistance{700};
    float steelThickness{10.f};

    std::vector<NDT::XrayDevice> ExposureRecalculation(const std::vector<NDT::XrayDevice> &deviceVector,
                                           int distance);
};