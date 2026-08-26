#include "mastersWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void MastersWindow::Show(std::vector<Master> &mastersList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс производителя СМР, который создаётся/редактируется

    if (ImGui::BeginTable("Производители СМР", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
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

        static std::vector<int> visibleIndices; /// индексы mastersList, проходящие фильтр удаления - static, чтобы не аллоцировать вектор заново каждый кадр
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (mastersList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
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
                    masterEditWindowIsOpen = true;
                    editingIndex = row;
                }
            }

            if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
            {
                if (ImGui::MenuItem("Редактировать"))
                {
                    masterEditWindowIsOpen = true;
                    editingIndex = row;
                }
                if (ImGui::MenuItem("Удалить"))
                    mastersList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            NDT::TextWithTooltipIfTruncated(mastersList.at(row).name);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(mastersList.at(row).organization);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(mastersList.at(row).department);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(mastersList.at(row).position);

            const bool dateMismatch = mastersList.at(row).certificateEndDate <= NDT::GetCurrentIsoDate();
            if (dateMismatch)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(mastersList.at(row).certificateNumber);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(mastersList.at(row).certificateEndDate));

            if (dateMismatch)
                ImGui::PopStyleColor();

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        mastersList.resize(tableRows);
        masterEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    if (masterEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(mastersList.size()))
        masterEditWindow.Show(mastersList.at(editingIndex), masterEditWindowIsOpen);
}
