#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "resourceManager.hpp"

class ApplicationData;

class NomogramWindow
{
public:
    explicit NomogramWindow(ApplicationData &coreAppData, ResourceManager &resManager);
    void Show();

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

    enum class Measure : uint8_t
    {
        mAxmin,
        min,
        sec
    };

    // схема просвечивания (ГОСТ ISO 17636-1, п. 7.1)
    enum class Scheme : uint8_t
    {
        Panoramic, // источник внутри трубы, рис. 5
        Frontal,   // источник снаружи, оценка стенки у детектора, рис. 13
        Elliptic,  // через две стенки на эллипс, рис. 11
        Plate,     // плоский объект, рис. 1
        Count
    };

    void DrawScheme(ImVec2 size);
    // панорамная схема с конусным аппаратом (торцевой конус, полусфера): пучок вдоль трубы
    bool IsConeInPipe() const;

    static constexpr float weldBulge = 3.f; // выпуклость шва, на неё же отодвинут детектор, мм
    static constexpr float frontalAngleMax = 5.f; // наклон к плоскости шва для фронтальной, градусы

    std::string nameAxisY = "Экспозиция, мА х мин";
    int deviceIndex = {0};
    Measure measureIndex = Measure::mAxmin;
    Scheme scheme = Scheme::Plate;
    float pipeDiameter = {159.f};
    float focusDistance = {700.f};
    float detectorWidth = {100.f}; // вдоль оси трубы (вдоль шва у пластины), мм
    float sourceShift = {50.f}; // смещение источника от плоскости шва (эллипс, фронтальная), мм
    float steelThickness = {10.f};
    float deviceCurrent = {1.f};
    float steelThicknessMin = {1.f};
    float steelThicknessMax = {10.f};
    std::vector<XrayDevice> devices;
    std::vector<XrayDevice> calculatedDevices;
};