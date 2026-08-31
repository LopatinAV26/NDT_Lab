#include "employeesWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "ImGuiDatePicker.hpp"

void EmployeesWindow::Show(std::vector<Employee> &emplList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс сотрудника, который создаётся/редактируется

    if (ImGui::BeginTable("Сотрудники", 16, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Имя");
        ImGui::TableSetupColumn("Организация");
        ImGui::TableSetupColumn("Подразделение");
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Дата\nтрудоустройства");
        ImGui::TableSetupColumn("Опыт\nработы");
        ImGui::TableSetupColumn("Личный\nкод");
        ImGui::TableSetupColumn("Разряд");
        ImGui::TableSetupColumn("Номер\nудостоверения");
        ImGui::TableSetupColumn("Дата выдачи\nудостоверения");
        ImGui::TableSetupColumn("Срокд ействия\nВИК");
        ImGui::TableSetupColumn("Срок действия\nУК");
        ImGui::TableSetupColumn("Срок действия\nРК");
        ImGui::TableSetupColumn("Срок действия\nПВК");
        ImGui::TableSetupColumn("Срок действия\nМК");
        ImGui::TableSetupColumn("Срок действия\nПВТ");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(emplList.size());

        static std::vector<int> visibleIndices;
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (emplList.at(row).deletedAt.has_value())
                continue;

            visibleIndices.push_back(row);
        }

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visibleIndices.size()));
        while (clipper.Step())
            for (int visRow = clipper.DisplayStart; visRow < clipper.DisplayEnd; ++visRow)
            {
                int row = visibleIndices[visRow];

                ImGui::TableNextRow();
                ImGui::PushID(row);

                const Employee &employeeRow = emplList.at(row);
                const std::string experienceStr = NDT::GetExperience(employeeRow.employeementDate);

                ImGui::TableSetColumnIndex(0);
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
                if (ImGui::Selectable("###", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap))
                {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        editWindow = true;
                        editingIndex = row;
                    }
                }

                if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
                {
                    if (ImGui::MenuItem("Редактировать"))
                    {
                        editWindow = true;
                        editingIndex = row;
                    }
                    if (ImGui::MenuItem("Удалить"))
                        emplList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                NDT::TextWithTooltipIfTruncated(employeeRow.name);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.organization);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.department);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.position);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(employeeRow.employeementDate));

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(experienceStr);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.personalCode);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.level);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.certificateNumber);

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(employeeRow.certificateDate));

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasVT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDateVT) : "метод отсутствует");

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasUT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDateUT) : "метод отсутствует");

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasRT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDateRT) : "метод отсутствует");

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasPT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDatePT) : "метод отсутствует");

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasMT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDateMT) : "метод отсутствует");

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(employeeRow.hasLT ? NDT::FormatDateForDisplay(employeeRow.certificateEndDateLT) : "метод отсутствует");

                ImGui::PopID();
            }
        ImGui::EndTable();
    }
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        emplList.resize(tableRows);
        editWindow = true;
        editingIndex = tableRows - 1;
    }

    if (editWindow && editingIndex >= 0 &&
        editingIndex < static_cast<int>(emplList.size()))
        Edit(emplList.at(editingIndex), editWindow);
}

void EmployeesWindow::Edit(Employee &empl, bool &isOpen)
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
            empl.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
