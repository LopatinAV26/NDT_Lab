#include "reportCreateWindow.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"

void ReportCreateWindow::Show(ReportData &report, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Новое заключение", &isOpen, window_flags))
    {
        ImGui::InputText("Дата выдачи заключения", &report.protocolDate);
        ImGui::InputText("Номер заключения", &report.protocolNumber);

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.ShowDefectCreateWindow(report, defectWindowIsOpen);
}
