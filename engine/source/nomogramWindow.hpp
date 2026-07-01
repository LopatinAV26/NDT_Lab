#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "resourceManager.hpp"

class ApplicationData;
//class ResourceManager;

class NomogramWindow
{
public:
    explicit NomogramWindow(ApplicationData &coreAppData, ResourceManager &resManager);
    void Show(bool &isOpen);

private:
    struct CurvesRef
    {
        const std::vector<float> x;
        const std::vector<float> y;
        ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
        std::string label;
    };

    std::vector<XrayDevice> ExposureRecalculation(const std::vector<XrayDevice> &deviceVector,
                                                  float distance, float current) const;

    void DrawMarkers(const std::vector<CurvesRef> &curves, float thickness) const;

    enum class Measure
    {
        mAxmin,
        min,
        sec
    };

    std::string nameAxisY = "Экспозиция, мА х мин";
    int deviceIndex = {0};
    Measure measureIndex = Measure::mAxmin;
    float focusDistance = {700.f};
    float steelThickness = {10.f};
    float deviceCurrent = {1.f};
    float steelThicknessMin = {1.f};
    float steelThicknessMax = {10.f};
    std::vector<XrayDevice> devices;
    std::vector<XrayDevice> calculatedDevices;
    ApplicationData &appData;
    ResourceManager &resMgr;
};