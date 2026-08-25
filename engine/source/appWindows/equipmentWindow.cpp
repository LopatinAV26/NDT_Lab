#include "equipmentWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"

void EquipmentWindow::Show(std::vector<Equipment> &equipmentList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс оборудования, которое создаётся/редактируется
    static std::string searchQuery;

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##equipmentSearch", "Поиск по наименованию...", &searchQuery);

    if (ImGui::BeginTable("Оборудование", 14, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Наименование");
        ImGui::TableSetupColumn("Метод контроля");
        ImGui::TableSetupColumn("Назначение");
        ImGui::TableSetupColumn("Производитель");
        ImGui::TableSetupColumn("Серийный номер");
        ImGui::TableSetupColumn("Год выпуска");
        ImGui::TableSetupColumn("Год ввода\nв эксплуатацию");
        ImGui::TableSetupColumn("Технические и метрологические\nхарактеристики");
        ImGui::TableSetupColumn("Владелец");
        ImGui::TableSetupColumn("Номер документа\nо поверке/калибровке");
        ImGui::TableSetupColumn("Дата документа\nо поверке/калибровке");
        ImGui::TableSetupColumn("Срок действия\nдокумента");
        ImGui::TableSetupColumn("Состояние");
        ImGui::TableSetupColumn("Файл\nсвидетельства");
        ImGui::TableSetupScrollFreeze(1, 1);
        ImGui::TableHeadersRow();

        tableRows = static_cast<int>(equipmentList.size());
        const std::string lowerQuery = searchQuery.empty() ? std::string() : NDT::ToLowerUtf8(searchQuery); /// приводим строку поиска к нижнему регистру один раз за кадр, а не на каждой строке

        static std::vector<int> visibleIndices; /// индексы equipmentList, проходящие фильтр удаления/поиска - static, чтобы не аллоцировать вектор заново каждый кадр
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (equipmentList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
                continue;

            if (!lowerQuery.empty() && NDT::ToLowerUtf8(equipmentList.at(row).name).find(lowerQuery) == std::string::npos)
                continue;

            visibleIndices.push_back(row);
        }

        /// только реально видимые строки вместо всех visibleIndices.size()
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(visibleIndices.size()));
        while (clipper.Step())
        for (int visRow = clipper.DisplayStart; visRow < clipper.DisplayEnd; ++visRow)
        {
            int row = visibleIndices[visRow];

            ImGui::TableNextRow();
            ImGui::PushID(row);

            ImGui::TableSetColumnIndex(0);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg)); /// подсвечиваем закреплённый столбец как шапку таблицы
            /// ImGuiSelectableFlags_AllowOverlap - иначе Selectable, растянутый на всю строку, перехватывает клики по ссылке в столбце "Файл"
            if (ImGui::Selectable("###", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowOverlap))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    equipmentEditWindowIsOpen = true;
                    editingIndex = row;
                }
            }

            if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
            {
                if (ImGui::MenuItem("Редактировать"))
                {
                    equipmentEditWindowIsOpen = true;
                    editingIndex = row;
                }
                if (ImGui::MenuItem("Удалить"))
                    equipmentList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).name);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).method);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).function);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).manufacturer);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).serialNumber);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).yearOfManufacture);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).yearOfCommissioning);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).technicalAndMetrologicalCharacteristics);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).owner);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).isCalibrated ? equipmentList.at(row).certificateNumber : std::string("не поверяется"));

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).isCalibrated ? NDT::FormatDateForDisplay(equipmentList.at(row).certificateDate) : std::string("-"));

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).isCalibrated ? NDT::FormatDateForDisplay(equipmentList.at(row).certificateEndDate) : std::string("-"));

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(equipmentList.at(row).state);

            ImGui::TableNextColumn();
            {
                Equipment &equipment = equipmentList.at(row);
                if (equipment.fileName.empty())
                    ImGui::TextUnformatted("-");
                else if (ImGui::TextLink(equipment.fileName.c_str()))
                    NDT::OpenFileFromBytes(equipment.fileName, equipment.fileData);
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        equipmentList.resize(tableRows);
        equipmentEditWindowIsOpen = true;
        editingIndex = tableRows - 1;
    }
    if (equipmentEditWindowIsOpen && editingIndex >= 0 &&
        editingIndex < static_cast<int>(equipmentList.size()))
        equipmentEditWindow.Show(equipmentList.at(editingIndex), equipmentEditWindowIsOpen);
}
