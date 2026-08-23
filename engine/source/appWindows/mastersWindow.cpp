#include "mastersWindow.hpp"

#include <cfloat>
#include <algorithm>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void MastersWindow::Show(std::vector<Master> &mastersList)
{
    static int tableRows = 0;            ///< количество строк в таблице
    static int editingIndex = -1;        ///< текущий индекс производителя СМР, который создаётся/редактируется
    static std::vector<int> indexesList; ///< Список индексов для печати в pdf
    static std::vector<bool> selected;

    if (ImGui::BeginTable("Производители СМР", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Имя");
        ImGui::TableSetupColumn("Организация");
        ImGui::TableSetupColumn("Подразделение");
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Номер\nудостоверения");
        ImGui::TableSetupColumn("Срок действия\nудостоверения", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(mastersList.size());
        selected.resize(tableRows);
        for (int row = 0; row < tableRows; ++row)
        {
            if (mastersList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(row);

            const Master &masterRow = mastersList.at(row);
            const std::string *cellTexts[] = {
                &masterRow.name, &masterRow.organization,
                &masterRow.department, &masterRow.position,
                &masterRow.certificateNumber, &masterRow.certificateEndDate};

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
                    masterEditWindowIsOpen = true;
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
            ImGui::TextUnformatted(std::format("{:s}", mastersList.at(row).name).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", mastersList.at(row).organization).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", mastersList.at(row).department).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", mastersList.at(row).position).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);

            const bool dateMismatch = mastersList.at(row).certificateEndDate <= NDT::GetCurrentIsoDate();
            if (dateMismatch)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

            ImGui::TextUnformatted(std::format("{:s}", mastersList.at(row).certificateNumber).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);

            ImGui::TextUnformatted(NDT::FormatDateForDisplay(mastersList.at(row).certificateEndDate).c_str());
            if (dateMismatch)
                ImGui::PopStyleColor();

            ImGui::PopTextWrapPos();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("+")) //////////////////////////////////////////
    {
        tableRows++;
        mastersList.resize(tableRows);
        masterEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(indexesList.empty());
    if (ImGui::Button("Удалить выбранные")) ////////////////////////////////////////
    {
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

        for (int idx : indexesList)
            mastersList.at(idx).deletedAt = now; /// помечаем время удаления

        indexesList.clear();
        std::ranges::fill(selected, false); // индексы после удаления сдвинулись, старые флаги уже не соответствуют строкам
    }
    ImGui::EndDisabled();

    if (masterEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(mastersList.size()))
        masterEditWindow.Show(mastersList.at(editingIndex), masterEditWindowIsOpen);
}
