#pragma once

#include "appWindows/nomogramWindow.hpp"
#include "appWindows/protocolWindow.hpp"

class ApplicationData;
class ResourceManager;

class LabWindow
{
public:
    LabWindow(ApplicationData &coreAppData, ResourceManager &resourceManager);
    void Show(bool &isOpen);

private:
    NomogramWindow nomogramWindow;
    ProtocolWindow reportWindow;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
};