#include "inspectorsWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "ImGuiDatePicker.hpp"

void InspectorsWindow::Show(std::vector<Inspector> &inspectorsList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс инспектора, который создаётся/редактируется

    if (ImGui::BeginTable("Сотрудники надзора", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
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

        static std::vector<int> visibleIndices; /// индексы inspectorsList, проходящие фильтр удаления - static, чтобы не аллоцировать вектор заново каждый кадр
        visibleIndices.clear();
        for (int row = 0; row < tableRows; ++row)
        {
            if (inspectorsList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
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
                    inspectorsList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            NDT::TextWithTooltipIfTruncated(inspectorsList.at(row).name);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(inspectorsList.at(row).organization);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(inspectorsList.at(row).certificateNumber);

            ImGui::TableNextColumn();
            NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateDate));

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasVT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateVT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasUT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateUT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasRT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateRT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasPT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDatePT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasMT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateMT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::TableNextColumn();
            if (inspectorsList.at(row).hasLT)
                NDT::TextWithTooltipIfTruncated(NDT::FormatDateForDisplay(inspectorsList.at(row).certificateEndDateLT));
            else
                ImGui::TextUnformatted("метод отсутствует");

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Добавить")) //////////////////////////////////////////
    {
        tableRows++;
        inspectorsList.resize(tableRows);
        editWindow = true;
        editingIndex = tableRows - 1;
    }
    if (editWindow && editingIndex >= 0 &&
        editingIndex < static_cast<int>(inspectorsList.size()))
        Edit(inspectorsList.at(editingIndex), editWindow);
}

void InspectorsWindow::Edit(Inspector &inspector, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Сотрудник надзора", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Ф.И.О.");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##name#", &inspector.name);

        ImGui::TextDisabled("Организация");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##organization#", &inspector.organization);

        ImGui::TextDisabled("Номер удостоверения/разрешения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##certificateNumber#", &inspector.certificateNumber);

        ImGui::TextDisabled("Дата выдачи удостоверения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateDate);
            if (ImGui::DatePicker("##certificateDate#", date))
            {
                inspector.certificateDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::SeparatorText("Методы НК");
        changed |= ImGui::Checkbox("ВИК", &inspector.hasVT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasVT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateVT);
            if (ImGui::DatePicker("##ВИК#", date))
            {
                inspector.certificateEndDateVT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("УК", &inspector.hasUT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasUT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateUT);
            if (ImGui::DatePicker("##УК#", date))
            {
                inspector.certificateEndDateUT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("РК", &inspector.hasRT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasRT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateRT);
            if (ImGui::DatePicker("##РК#", date))
            {
                inspector.certificateEndDateRT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВК", &inspector.hasPT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasPT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDatePT);
            if (ImGui::DatePicker("##ПВК#", date))
            {
                inspector.certificateEndDatePT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("МК", &inspector.hasMT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasMT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateMT);
            if (ImGui::DatePicker("##МК#", date))
            {
                inspector.certificateEndDateMT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        changed |= ImGui::Checkbox("ПВТ", &inspector.hasLT);
        ImGui::SameLine();
        ImGui::BeginDisabled(!inspector.hasLT);
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(inspector.certificateEndDateLT);
            if (ImGui::DatePicker("##ПВТ#", date))
            {
                inspector.certificateEndDateLT = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }
        ImGui::EndDisabled();

        if (changed)
            inspector.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
