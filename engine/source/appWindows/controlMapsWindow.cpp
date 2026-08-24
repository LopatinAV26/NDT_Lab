#include "controlMapsWindow.hpp"

#include <cfloat>
#include <algorithm>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void ControlMapsWindow::Show(std::vector<ControlMap> &controlMapsList)
{
    static int tableRows = 0;            ///< количество строк в таблице
    static int editingIndex = -1;        ///< текущий индекс техкарты, которая создаётся/редактируется
    static std::vector<int> indexesList; ///< Список индексов для печати в pdf
    static std::vector<bool> selected;

    if (ImGui::BeginTable("Технологические карты", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Шифр");
        ImGui::TableSetupColumn("Метод\nконтроля");
        ImGui::TableSetupColumn("Диаметр");
        ImGui::TableSetupColumn("Толщина\nстенки");
        ImGui::TableSetupColumn("Описание");
        ImGui::TableSetupColumn("Файл");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(controlMapsList.size());
        selected.resize(tableRows);
        for (int row = 0; row < tableRows; ++row)
        {
            if (controlMapsList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            ImGui::TableNextRow();
            ImGui::PushID(row);

            const ControlMap &controlMapRow = controlMapsList.at(row);
            const std::string *cellTexts[] = {
                &controlMapRow.code, &controlMapRow.method, &controlMapRow.diameter,
                &controlMapRow.thickness, &controlMapRow.description, &controlMapRow.fileName};

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
            /// ImGuiSelectableFlags_AllowOverlap - иначе Selectable, растянутый на всю строку, перехватывает клики по ссылке в столбце "Файл"
            if (ImGui::Selectable("###", selected.at(row), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap, ImVec2(0, rowHeight)))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    controlMapEditWindowIsOpen = true;
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
            ImGui::TextUnformatted(std::format("{:s}", controlMapsList.at(row).code).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", controlMapsList.at(row).method).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", controlMapsList.at(row).diameter).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", controlMapsList.at(row).thickness).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            ImGui::TextUnformatted(std::format("{:s}", controlMapsList.at(row).description).c_str());
            ImGui::PopTextWrapPos();

            ImGui::TableNextColumn();
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
            {
                ControlMap &controlMap = controlMapsList.at(row);
                if (controlMap.fileName.empty())
                    ImGui::TextUnformatted("(не прикреплён)");
                else if (ImGui::TextLink(controlMap.fileName.c_str()))
                    NDT::OpenFileFromBytes(controlMap.fileName, controlMap.fileData);
            }
            ImGui::PopTextWrapPos();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("+")) //////////////////////////////////////////
    {
        tableRows++;
        controlMapsList.resize(tableRows);
        controlMapEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(indexesList.empty());
    if (ImGui::Button("Удалить выбранные")) ////////////////////////////////////////
    {
        auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

        for (int idx : indexesList)
            controlMapsList.at(idx).deletedAt = now; /// помечаем время удаления

        indexesList.clear();
        std::ranges::fill(selected, false); // индексы после удаления сдвинулись, старые флаги уже не соответствуют строкам
    }
    ImGui::EndDisabled();

    if (controlMapEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(controlMapsList.size()))
        controlMapEditWindow.Show(controlMapsList.at(editingIndex), controlMapEditWindowIsOpen);
}
