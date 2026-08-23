#include "weldersEditWindow.hpp"

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"

void WeldersEditWindow::Show(Welder &welder, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Сварщик", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Ф.И.О.");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##name#", &welder.name);

        ImGui::TextDisabled("Организация");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##organization#", &welder.organization);

        ImGui::TextDisabled("Подразделение");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##department#", &welder.department);

        ImGui::TextDisabled("Должность");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##position#", &welder.position);

        ImGui::TextDisabled("Шифр клейма");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##personalCode#", &welder.personalCode);

        ImGui::TextDisabled("Номер удостоверения/разрешения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##certificateNumber#", &welder.certificateNumber);

        ImGui::TextDisabled("Срок действия удостоверения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(welder.certificateEndDate);
            if (ImGui::DatePicker("##certificateEndDate#", date))
            {
                welder.certificateEndDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        if (changed)
            welder.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
