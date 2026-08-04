#include "employeeEditingWindow.hpp"

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"

void EmployeeEditingWindow::Show(Employee &empl, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Сотрудник", &isOpen, window_flags))
    {
        bool changed = false;

        // ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 0.0f, 0.15f));

        ImGui::TextDisabled("Ф.И.О.");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##name#", &empl.name);

        ImGui::TextDisabled("Организация");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##organization#", &empl.organization);

        ImGui::TextDisabled("Подразделение");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##department#", &empl.department);

        ImGui::TextDisabled("Должность");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##position#", &empl.position);

        ImGui::TextDisabled("Дата трудоустройства");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.employeementDate);
            if (ImGui::DatePicker("##employeementDate#", date))
            {
                empl.employeementDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Личный код");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##personalCode#", &empl.personalCode);

        ImGui::TextDisabled("Разряд");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##level#", &empl.level);

        ImGui::TextDisabled("Номер удостоверения/разрешения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##certificateNumber#", &empl.certificateNumber);

        ImGui::TextDisabled("Дата выдачи удостоверения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateDate);
            if (ImGui::DatePicker("##certificateDate#", date))
            {
                empl.certificateDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::SeparatorText("Методы НК");
        changed |= ImGui::Checkbox("ВИК", &empl.hasVT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasVT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateVT);
            if (ImGui::DatePicker("##ВИК#", date))
            {
                empl.certificateEndDateVT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("УК", &empl.hasUT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasUT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateUT);
            if (ImGui::DatePicker("##УК#", date))
            {
                empl.certificateEndDateUT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("РК", &empl.hasRT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasRT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateRT);
            if (ImGui::DatePicker("##РК#", date))
            {
                empl.certificateEndDateRT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВК", &empl.hasPT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasPT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDatePT);
            if (ImGui::DatePicker("##ПВК#", date))
            {
                empl.certificateEndDatePT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("МК", &empl.hasMT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasMT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateMT);
            if (ImGui::DatePicker("##МК#", date))
            {
                empl.certificateEndDateMT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВТ", &empl.hasLT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!empl.hasLT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateLT);
            if (ImGui::DatePicker("##ПВТ#", date))
            {
                empl.certificateEndDateLT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        // ImGui::PopStyleColor();

        if (changed)
        {
            empl.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        }
    }
    ImGui::End();
}
