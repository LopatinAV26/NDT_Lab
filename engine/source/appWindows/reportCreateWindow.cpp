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

    const std::string windowTitle = "Заключение " + NDT::ToLowerUtf8(report.method);

    if (ImGui::Begin(windowTitle.c_str(), &isOpen, window_flags))
    {
        bool changed = false;

        if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("1");
            ImGui::TableSetupColumn("2");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::TextDisabled("Дата проведения контроля");
            {
                tm date = NDT::ParseIsoDateTm(report.controlDate);
                ImGui::SetNextItemWidth(-FLT_MIN);
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
                ImGui::SetNextItemWidth(-FLT_MIN);
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
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Номер сварного соединения#", &report.weldNumber);

            ImGui::TextDisabled("Номер заключения");
            ImGui::SameLine();
            NDT::HelpMarker("");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Номер заключения#", &report.reportNumber);

            ImGui::TextDisabled("Наименование объекта");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Наименование объекта#", &report.objectName);

            ImGui::TextDisabled("Категория трубопровода");
            ImGui::SetNextItemWidth(-FLT_MIN);
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

            ImGui::TextDisabled("Подрядная организация");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Подрядная организация#", &report.contractorOrganization);

            ImGui::TextDisabled("Организация заказчика");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Организация заказчика#", &report.customerOrganization);

            ImGui::TextDisabled("Контроль произвёл");
            ImGui::SetNextItemWidth(-FLT_MIN);
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

            ImGui::TextDisabled("Заключение выдал");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Заключение выдал#", report.protocolCreateName.c_str())) ////////////////////////////////////////
            {
                for (const auto &employee : lab.employeesList)
                {
                    if (employee.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.protocolCreateName == employee.name);
                    if (ImGui::Selectable(employee.name.c_str(), isSelected))
                    {
                        report.protocolCreateName = employee.name;
                        report.protocolCreateOrganization = employee.organization;
                        report.protocolCreateCertNumber = employee.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Надзор");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Надзор#", report.inspectorName.c_str())) ////////////////////////////////////////
            {
                const bool noneSelected = report.inspectorName.empty();
                if (ImGui::Selectable("—", noneSelected))
                {
                    report.inspectorName.clear();
                    report.inspectorOrganization.clear();
                    report.inspectorCertNumber.clear();
                    changed = true;
                }

                if (noneSelected)
                    ImGui::SetItemDefaultFocus();

                for (const auto &inspector : lab.inspectorsList)
                {
                    if (inspector.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.inspectorName == inspector.name);
                    if (ImGui::Selectable(inspector.name.c_str(), isSelected))
                    {
                        report.inspectorName = inspector.name;
                        report.inspectorOrganization = inspector.organization;
                        report.inspectorCertNumber = inspector.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Мастер");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Мастер#", report.masterName.c_str())) ////////////////////////////////////////
            {
                const bool noneSelected = report.masterName.empty();
                if (ImGui::Selectable("—", noneSelected))
                {
                    report.masterName.clear();
                    report.masterOrganization.clear();
                    report.masterCertNumber.clear();
                    changed = true;
                }

                if (noneSelected)
                    ImGui::SetItemDefaultFocus();

                for (const auto &master : lab.mastersList)
                {
                    if (master.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.masterName == master.name);
                    if (ImGui::Selectable(master.name.c_str(), isSelected))
                    {
                        report.masterName = master.name;
                        report.masterOrganization = master.organization;
                        report.masterCertNumber = master.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::EndTable();
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
