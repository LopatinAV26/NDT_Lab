#include "normativeDocumentsWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void NormativeDocumentsWindow::Show(std::vector<NormativeDocument> &normativeDocumentsList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс документа, который создаётся/редактируется

    if (ImGui::BeginTable("Нормативные документы", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Шифр");
        ImGui::TableSetupColumn("Наименование");
        ImGui::TableSetupColumn("Метод\nконтроля");
        ImGui::TableSetupColumn("Статус");
        ImGui::TableSetupColumn("Год");
        ImGui::TableSetupColumn("Файл");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(normativeDocumentsList.size());

        static std::vector<int> visibleIndices; /// индексы normativeDocumentsList, проходящие фильтр удаления - static, чтобы не аллоцировать вектор заново каждый кадр
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (normativeDocumentsList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
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
                    normativeDocumentEditWindowIsOpen = true;
                    editingIndex = row;
                }
            }

            if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
            {
                if (ImGui::MenuItem("Редактировать"))
                {
                    normativeDocumentEditWindowIsOpen = true;
                    editingIndex = row;
                }
                if (ImGui::MenuItem("Удалить"))
                    normativeDocumentsList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            NDT::TextWithTooltipIfTruncated(normativeDocumentsList.at(row).code);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(normativeDocumentsList.at(row).name);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(normativeDocumentsList.at(row).method);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(normativeDocumentsList.at(row).status);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(normativeDocumentsList.at(row).year);

            ImGui::TableNextColumn();
            {
                NormativeDocument &normativeDocument = normativeDocumentsList.at(row);
                if (normativeDocument.fileName.empty())
                    ImGui::TextUnformatted("(не прикреплён)");
                else if (ImGui::TextLink(normativeDocument.fileName.c_str()))
                    NDT::OpenFileFromBytes(normativeDocument.fileName, normativeDocument.fileData);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        normativeDocumentsList.resize(tableRows);
        normativeDocumentEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    if (normativeDocumentEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(normativeDocumentsList.size()))
        normativeDocumentEditWindow.Show(normativeDocumentsList.at(editingIndex), normativeDocumentEditWindowIsOpen);
}
