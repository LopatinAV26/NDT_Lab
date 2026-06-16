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
    void SetGuiStyle() const;

    float fpsUpdateTimer{0.f};
    float currentFrametime{0.f};
    float framerate{0.f};
    ApplicationData &appData;
};

struct CurvesRef
{
    const std::vector<float> x;
    const std::vector<float> y;
    ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    std::string label;
};

class NomogramWindow
{
public:
    explicit NomogramWindow(ApplicationData &coreAppData);
    void Show(bool &isOpen);

private:
    std::vector<NDT::XrayDevice> ExposureRecalculation(const std::vector<NDT::XrayDevice> &deviceVector,
                                                       float distance, float current) const;

    void DrawMarkers(const std::vector<CurvesRef> &curves, float thickness) const;

    int deviceIndex{0};
    int measurementUnits_index{0};
    float focusDistance{700.f};
    float steelThickness{10.f};
    float deviceCurrent{1.f};
    std::vector<NDT::XrayDevice> devices;
    std::vector<NDT::XrayDevice> calculatedDevices;
    ApplicationData &appData;
};