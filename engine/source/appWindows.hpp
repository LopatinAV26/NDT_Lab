#pragma once

#include <memory>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "implot.h"

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
