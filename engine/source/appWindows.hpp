#pragma once

#include <memory>
#include <vector>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "implot.h"

#include "applicationData.hpp"
#include "xrayDevice.hpp"

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

class NomogramWindow{
public:
    explicit NomogramWindow();
    void Show(bool &isOpen);

private:
    std::string plotTitle{"Диаграмма экспозиции РА"};
    std::string nameAxisY{""};
    std::string xrayInfo{""};

    //std::vector<std::unique_ptr<XrayBase>> xray;
};