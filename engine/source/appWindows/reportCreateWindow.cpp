#include "reportCreateWindow.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"
#include "laboratory.hpp"

void ReportCreateWindow::Show(Report &report, bool &isOpen, Laboratory &lab)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Новое заключение", &isOpen, window_flags))
    {
        bool changed = false;

        {
            tm date = NDT::ParseIsoDateTm(report.protocolDate);
            if (ImGui::DatePicker("##Дата выдачи заключения#", date))///////////////////////
            {
                report.protocolDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Номер сварного соединения");
        changed |= ImGui::InputText("##Номер сварного соединения#", &report.weldNumber);//////////////

        ImGui::TextDisabled("Контроль произвёл");
        if (ImGui::BeginCombo("##Контроль произвёл#", report.controllerName.c_str())) ////////////////////////////////////////
        {
            for (const auto &employee : lab.employeesList)
            {
                if (employee.deletedAt.has_value())
                    continue;

                const bool isSelected = (report.controllerName == employee.name);
                if (ImGui::Selectable(employee.name.c_str(), isSelected))
                {
                    report.controllerName = employee.name;
                    report.controllerOrganization = employee.organization;
                    report.controllerCertNumber = employee.certificateNumber;
                    changed = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();

            if (changed)
            {
                report.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
            }
        }

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.Show(report, defectWindowIsOpen);
}
