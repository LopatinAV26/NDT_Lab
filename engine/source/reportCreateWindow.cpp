#include "reportCreateWindow.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

void ReportCreateWindow::Show(ReportData &report, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags =
        //  ImGuiWindowFlags_NoDecoration |
        //  ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Новое заключение", &isOpen, window_flags))
    {
        ImGui::InputText("Дата выдачи заключения", &report.protocolDate);
        ImGui::InputText("Номер заключения", &report.protocolNumber);

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.Show(report, defectWindowIsOpen);
}
