#pragma once

#include "laboratory.hpp"
#include "appWindows/nomogramWindow.hpp"
#include "appWindows/reportWindow.hpp"
#include "appWindows/employeesWindow.hpp"
#include "appWindows/otherEmployeesWindow.hpp"

class ApplicationData;
class ResourceManager;

class LabWindow
{
public:
    LabWindow(ApplicationData &coreAppData, ResourceManager &resourceManager);
    void Show(bool &isOpen);

    void ShowMain();

private:
    Laboratory lab;
    NomogramWindow nomogramWindow;
    ReportWindow reportWindow;
    EmployeesWindow emplWindow;
    OtherEmployeesWindow otherEmplWindow;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
    bool editingLabInfo = false;
};