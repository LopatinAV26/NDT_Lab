#pragma once

#include "laboratory.hpp"
#include "appWindows/nomogramWindow.hpp"
#include "appWindows/reportWindow.hpp"

class ApplicationData;
class ResourceManager;

class LabWindow
{
public:
    LabWindow(ApplicationData &coreAppData, ResourceManager &resourceManager);
    void Show(bool &isOpen);

private:
    Laboratory lab;
    NomogramWindow nomogramWindow;
    ReportWindow reportWindow;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
};