#include "otherEmployeesWindow.hpp"

#include <cfloat>
#include <algorithm>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void OtherEmployeesWindow::Show(std::vector<Inspector> &inspectorsList)
{
    static int tableRows = 0;            ///< количество строк в таблице
    static int editingIndex = -1;        ///< текущий индекс инспектора, который создаётся/редактируется
    static std::vector<int> indexesList; ///< Список индексов для печати в pdf
    static std::vector<bool> selected;

    if (ImGui::BeginTable("Сотрудники надзора", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Имя");
        ImGui::TableSetupColumn("Организация");
        ImGui::TableSetupColumn("Номер\nудостоверения");
        ImGui::TableSetupColumn("Дата выдачи\nудостоверения");
        ImGui::TableSetupColumn("Срок действия\nВИК");
        ImGui::TableSetupColumn("Срок действия\nУК");
        ImGui::TableSetupColumn("Срок действия\nРК");
        ImGui::TableSetupColumn("Срок действия\nПВК");
        ImGui::TableSetupColumn("Срок действия\nМК");
        ImGui::TableSetupColumn("Срок действия\nПВТ");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(inspectorsList.size());
        selected.resize(tableRows);
        for (int row = 0; row < tableRows; ++row)
        {
            if (inspectorsList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(row);

            const Inspector &inspectorRow = inspectorsList.at(row);
            const std::string *cellTexts[] = {
                &inspectorRow.name, &inspectorRow.organization,
                &inspectorRow.certificateNumber, &inspectorRow.certificateDate,
                &inspectorRow.certificateEndDateVT, &inspectorRow.certificateEndDateUT, &inspectorRow.certificateEndDateRT,
                &inspectorRow.certificateEndDatePT, &inspectorRow.certificateEndDateMT, &inspectorRow.certificateEndDateLT};

            // первый проход - только измеряем нужную высоту строки, ничего не рисуем
            float rowHeight = 0.0f;
            for (int col = 0; col < IM_ARRAYSIZE(cellTexts); ++col)
            {
                ImGui::TableSetColumnIndex(col);
                float colWidth = ImGui::GetContentRegionAvail().x;
                rowHeight = std::max(rowHeight, ImGui::CalcTextSize(cellTexts[col]->c_str(), nullptr, false, colWidth).y);
            }

            ImGui::TableSetColumnIndex(0); // возвращаемся к первому столбцу для настоящей отрисовки
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg)); /// подсвечиваем закреплённый столбец как шапку таблицы
            if (ImGui::Selectable("###", selected.at(row), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, rowHeight)))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    inspectorEditWindowIsOpen = true;
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
            ImGui::TextUnformatted(std::format("{:s}", inspectorsList.at(row).name).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", inspectorsList.at(row).organization).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", inspectorsList.at(row).certificateNumber).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateDate).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasVT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateVT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasUT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateUT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasRT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateRT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasPT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDatePT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasMT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateMT).c_str());
            else
                ImGui::Text("метод отсутствует");
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            if (inspectorsList.at(row).hasLT)
                ImGui::TextUnformatted(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateLT).c_str());
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
        inspectorsList.resize(tableRows);
        inspectorEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(indexesList.empty());
    if (ImGui::Button("Удалить выбранные")) ////////////////////////////////////////
    {
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

        for (int idx : indexesList)
            inspectorsList.at(idx).deletedAt = now; /// помечаем время удаления

        indexesList.clear();
        std::ranges::fill(selected, false); // индексы после удаления сдвинулись, старые флаги уже не соответствуют строкам
    }
    ImGui::EndDisabled();

    if (inspectorEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(inspectorsList.size()))
        inspectorEditWindow.Show(inspectorsList.at(editingIndex), inspectorEditWindowIsOpen);
}
