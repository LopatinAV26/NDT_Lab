#include "employeeEditingWindow.hpp"

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"

void EmployeeEditingWindow::Show(Employee &empl, bool &isOpen)
{
    const char *popupId = "Сотрудник";

    if (isOpen)
        ImGui::OpenPopup(popupId);

    if (ImGui::BeginPopupModal(popupId, &isOpen, window_flags))
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

        ImGui::TextDisabled("Дата истечения ВИК");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateVT);
            if (ImGui::DatePicker("##Дата истечения ВИК#", date))
            {
                empl.certificateEndDateVT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата истечения УК");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateUT);
            if (ImGui::DatePicker("##Дата истечения УК#", date))
            {
                empl.certificateEndDateUT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата истечения РК");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateRT);
            if (ImGui::DatePicker("##Дата истечения РК#", date))
            {
                empl.certificateEndDateRT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата истечения ПВК");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDatePT);
            if (ImGui::DatePicker("##Дата истечения ПВК#", date))
            {
                empl.certificateEndDatePT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата истечения МК");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateMT);
            if (ImGui::DatePicker("##Дата истечения МК#", date))
            {
                empl.certificateEndDateMT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата истечения ПВТ");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(empl.certificateEndDateLT);
            if (ImGui::DatePicker("##Дата истечения ПВТ#", date))
            {
                empl.certificateEndDateLT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        // ImGui::PopStyleColor();

        if (changed)
        {
            empl.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        }

        ImGui::EndPopup();
    }
}
