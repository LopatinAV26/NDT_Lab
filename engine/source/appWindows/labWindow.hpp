#pragma once

#include "laboratory.hpp"
#include "appWindows/nomogramWindow.hpp"
#include "appWindows/reportWindow.hpp"
#include "appWindows/employeesWindow.hpp"
#include "appWindows/inspectorsWindow.hpp"
#include "appWindows/mastersWindow.hpp"
#include "appWindows/weldersWindow.hpp"
#include "appWindows/equipmentWindow.hpp"
#include "appWindows/controlMapsWindow.hpp"
#include "appWindows/normativeDocumentsWindow.hpp"

class ApplicationData;
class ResourceManager;

class LabWindow
{
public:
    LabWindow(ApplicationData &coreAppData, ResourceManager &resourceManager);
    ~LabWindow();
    
    void Show(bool &isOpen);

    void ShowMain();

private:
    Laboratory lab;
    NomogramWindow nomogramWindow;
    ReportWindow reportWindow;
    EmployeesWindow emplWindow;
    InspectorsWindow inspectorsWindow;
    MastersWindow mastersWindow;
    WeldersWindow weldersWindow;
    EquipmentWindow equipmentWindow;
    ControlMapsWindow controlMapsWindow;
    NormativeDocumentsWindow normativeDocumentsWindow;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
    bool editingLabInfo = false;
};