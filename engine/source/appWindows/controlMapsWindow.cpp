#include "controlMapsWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void ControlMapsWindow::Show(std::vector<ControlMap> &controlMapsList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс техкарты, которая создаётся/редактируется

    if (ImGui::BeginTable("Технологические карты", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
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

        static std::vector<int> visibleIndices; /// индексы controlMapsList, проходящие фильтр удаления - static, чтобы не аллоцировать вектор заново каждый кадр
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (controlMapsList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            visibleIndices.push_back(row);
        }

        ImGuiListClipper clipper; /// измеряем и рисуем только реально видимые строки
        clipper.Begin(static_cast<int>(visibleIndices.size()));
        while (clipper.Step())
        for (int visRow = clipper.DisplayStart; visRow < clipper.DisplayEnd; ++visRow)
        {
            int row = visibleIndices[visRow];

            ImGui::TableNextRow();
            ImGui::PushID(row);

            ImGui::TableSetColumnIndex(0);                                                                 // возвращаемся к первому столбцу для настоящей отрисовки
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg)); /// подсвечиваем закреплённый столбец как шапку таблицы
            /// ImGuiSelectableFlags_AllowOverlap - иначе Selectable, растянутый на всю строку, перехватывает клики по ссылке в столбце "Файл"
            if (ImGui::Selectable("###", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    controlMapEditWindowIsOpen = true;
                    editingIndex = row;
                }
            }

            if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
            {
                if (ImGui::MenuItem("Редактировать"))
                {
                    controlMapEditWindowIsOpen = true;
                    editingIndex = row;
                }
                if (ImGui::MenuItem("Удалить"))
                    controlMapsList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).code);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).method);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).diameter);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).thickness);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).description);

            ImGui::TableNextColumn();
            {
                ControlMap &controlMap = controlMapsList.at(row);
                if (controlMap.fileName.empty())
                    ImGui::TextUnformatted("(не прикреплён)");
                else if (ImGui::TextLink(controlMap.fileName.c_str()))
                    NDT::OpenFileFromBytes(controlMap.fileName, controlMap.fileData);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        controlMapsList.resize(tableRows);
        controlMapEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    if (controlMapEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(controlMapsList.size()))
        controlMapEditWindow.Show(controlMapsList.at(editingIndex), controlMapEditWindowIsOpen);
}
