#include "employeesWindow.hpp"

#include <cfloat>
#include <algorithm>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void EmployeesWindow::Show(std::vector<Employee> &emplList)
{
    static int tableRows = 0;            ///< количество строк в таблице отчётов
    static int editingIndex = -1;        ///< текущий индекс отчёта, который создаётся/редактируется
    static std::vector<int> indexesList; ///< Список индексов для печати в pdf
    static std::vector<bool> selected;

    if (ImGui::BeginTable("Сотрудники", 16, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
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
        selected.resize(tableRows);
        for (int row = 0; row < tableRows; ++row)
        {
            if (emplList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(row);

            const Employee &employeeRow = emplList.at(row);
            const std::string experienceStr = NDT::GetExperience(employeeRow.employeementDate); /// стаж не хранится в БД, считается от даты трудоустройства
            const std::string *cellTexts[] = {
                &employeeRow.name, &employeeRow.organization, &employeeRow.department, &employeeRow.position,
                &employeeRow.employeementDate, &experienceStr, &employeeRow.personalCode, &employeeRow.level,
                &employeeRow.certificateNumber, &employeeRow.certificateDate, &employeeRow.certificateEndDateVT, &employeeRow.certificateEndDateUT,
                &employeeRow.certificateEndDateRT, &employeeRow.certificateEndDatePT, &employeeRow.certificateEndDateMT,
                &employeeRow.certificateEndDateLT};

            // первый проход - только измеряем нужную высоту строки, ничего не рисуем
            float rowHeight = 0.0f;
            for (int col = 0; col < IM_ARRAYSIZE(cellTexts); ++col)
            {
                ImGui::TableSetColumnIndex(col);
                float colWidth = ImGui::GetContentRegionAvail().x;
                rowHeight = std::max(rowHeight, ImGui::CalcTextSize(cellTexts[col]->c_str(), nullptr, false, colWidth).y);
            }

            ImGui::TableSetColumnIndex(0);                                                                 // возвращаемся к первому столбцу для настоящей отрисовки
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg)); /// подсвечиваем закреплённый столбец как шапку таблицы
            if (ImGui::Selectable("###", selected.at(row), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, rowHeight)))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    employeeEditWindowIsOpen = true;
                    editingIndex = row;
                }
                else
                {
                    selected.at(row) = !selected.at(row);
                    if (selected.at(row))
                        indexesList.push_back(row);
                    else
                        std::erase(indexesList, row);
                }
            }
            ImGui::SameLine();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).name).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).organization).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).department).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).position).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).employeementDate).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(experienceStr.c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).personalCode).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).level).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", emplList.at(row).certificateNumber).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateDate).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);

            if (emplList.at(row).hasVT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDateVT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);

            if (emplList.at(row).hasUT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDateUT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);

            if (emplList.at(row).hasRT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDateRT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (emplList.at(row).hasPT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDatePT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (emplList.at(row).hasMT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDateMT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (emplList.at(row).hasLT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(emplList.at(row).certificateEndDateLT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("+")) //////////////////////////////////////////
    {
        tableRows++;
        emplList.resize(tableRows);
        employeeEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(indexesList.empty());
    if (ImGui::Button("Удалить выбранные")) ////////////////////////////////////////
    {
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

        for (int idx : indexesList)
            emplList.at(idx).deletedAt = now; /// помечаем время удаления

        indexesList.clear();
        std::ranges::fill(selected, false); // индексы после удаления сдвинулись, старые флаги уже не соответствуют строкам
    }
    ImGui::EndDisabled();

    if (employeeEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(emplList.size()))
        emplEditWindow.Show(emplList.at(editingIndex), employeeEditWindowIsOpen);
}
