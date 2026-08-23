#include "inspectorsEditWindow.hpp"

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"

void InspectorsEditWindow::Show(Inspector &inspector, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Сотрудник надзора", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Ф.И.О.");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##name#", &inspector.name);

        ImGui::TextDisabled("Организация");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##organization#", &inspector.organization);

        ImGui::TextDisabled("Номер удостоверения/разрешения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##certificateNumber#", &inspector.certificateNumber);

        ImGui::TextDisabled("Дата выдачи удостоверения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateDate);
            if (ImGui::DatePicker("##certificateDate#", date))
            {
                inspector.certificateDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::SeparatorText("Методы НК");
        changed |= ImGui::Checkbox("ВИК", &inspector.hasVT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasVT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateVT);
            if (ImGui::DatePicker("##ВИК#", date))
            {
                inspector.certificateEndDateVT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("УК", &inspector.hasUT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasUT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateUT);
            if (ImGui::DatePicker("##УК#", date))
            {
                inspector.certificateEndDateUT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("РК", &inspector.hasRT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasRT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateRT);
            if (ImGui::DatePicker("##РК#", date))
            {
                inspector.certificateEndDateRT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВК", &inspector.hasPT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasPT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDatePT);
            if (ImGui::DatePicker("##ПВК#", date))
            {
                inspector.certificateEndDatePT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("МК", &inspector.hasMT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasMT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateMT);
            if (ImGui::DatePicker("##МК#", date))
            {
                inspector.certificateEndDateMT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВТ", &inspector.hasLT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasLT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateLT);
            if (ImGui::DatePicker("##ПВТ#", date))
            {
                inspector.certificateEndDateLT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        if (changed)
            inspector.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
