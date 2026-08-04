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

        ImGui::TextDisabled("Дата проведения контроля");
        {
            tm date = NDT::ParseIsoDateTm(report.controlDate);
            if (ImGui::DatePicker("##Дата проведения контроля#", date))
            {
                report.controlDate = NDT::FormatIsoDateTm(date);
                report.reportDate = report.controlDate;
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата выдачи заключения");
        ImGui::SameLine();
        const bool dateMismatch = report.reportDate != report.controlDate;
        if (dateMismatch)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        NDT::HelpMarker("Дата на заключении по результатам НК должна соответствовать фактической дате проведения контроля");
        {
            tm date = NDT::ParseIsoDateTm(report.reportDate);

            if (ImGui::DatePicker("##Дата выдачи заключения#", date))
            {
                report.reportDate = NDT::FormatIsoDateTm(date);
                if (report.reportDate < report.controlDate)
                    report.reportDate = report.controlDate;
                changed = true;
            }
            if (dateMismatch)
                ImGui::PopStyleColor();
        }

        ImGui::TextDisabled("Номер сварного соединения");
        changed |= ImGui::InputText("##Номер сварного соединения#", &report.weldNumber);

        ImGui::TextDisabled("Номер заключения");
        ImGui::SameLine();
        NDT::HelpMarker("");
        ImGui::InputText("##Номер заключения#", &report.reportNumber);

        ImGui::TextDisabled("Наименование объекта");
        ImGui::InputText("##Наименование объекта#", &report.objectName);

        ImGui::TextDisabled("Категория трубопровода");
        if (ImGui::BeginCombo("##Категория трубопровода#", report.pipeCategory.c_str()))
        {
            for (const auto &category : report.pipeCategoryList)
            {
                const bool isSelected = (report.pipeCategory == category);
                if (ImGui::Selectable(category.c_str(), isSelected))
                {
                    report.pipeCategory = category;
                    changed = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        



        

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
        }

        if (changed)
            {
                report.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
            }

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.Show(report, defectWindowIsOpen);
}
