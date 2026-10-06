#include "defectCreateWindow.hpp"

#include <algorithm>
#include <cfloat>
#include <format>
#include "laboratory.hpp"
#include "imgui_stdlib.h"

namespace
{
/// @brief Поле размера дефекта. Шаг кнопок меняется на границе 3 мм, как и шаг округления,
/// а само округление по РД делается, когда ввод закончен - иначе число правилось бы во время набора
/// @return значение изменилось
bool DefectSizeInput(const char *label, float &value)
{
    const float step = value <= 3.f + 1e-3f ? 0.1f : 0.5f;
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool edited = ImGui::InputFloat(label, &value, step, step, "%.1f");
    if (ImGui::IsItemDeactivatedAfterEdit())
        value = RoundDefectSize(value);
    return edited;
}
}

void DefectCreateWindow::Show(Report &report, bool &isOpen, bool isUnsaved)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Конструктор дефектов", &isOpen, window_flags | (isUnsaved ? ImGuiWindowFlags_UnsavedDocument : 0)))
    {
        bool changed = false; ///< правка дефектов, примечаний или результата - всё это данные заключения

        const int tableRows = static_cast<int>(report.defRGCList.size());
        int removeRow = -1;

        /// на эллипс дефект привязывается к экспозиции, а не к координате мерного пояса
        const bool isEllipse = report.exposureScheme == ExposureScheme::Ellipse;

        /// рядом с координатой показываем строку заключения, куда попадёт дефект; на эллипс её выбирают в первой колонке
        const int columnCount = isEllipse ? 8 : 9;

        if (ImGui::BeginTable("Defect creator", columnCount, ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn(isEllipse ? "Экспозиция" : "Координата");
            if (!isEllipse)
                ImGui::TableSetupColumn(report.GetSectionTitle().c_str());
            ImGui::TableSetupColumn("Обозначение");
            ImGui::TableSetupColumn("Протяжённость");
            ImGui::TableSetupColumn("Длина");
            ImGui::TableSetupColumn("Ширина");
            ImGui::TableSetupColumn("Превышение\nплотности");
            ImGui::TableSetupColumn("Допустим");
            ImGui::TableSetupColumn("##Удалить", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            for (int row = 0; row < tableRows; ++row)
            {
                DefectRt &def = report.defRGCList.at(row);
                const DefectRtFields fields = GetDefectRtFields(def.symbol);
                bool rowChanged = false;

                ImGui::TableNextRow();
                ImGui::PushID(row);

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (isEllipse)
                {
                    const int section = report.GetDefectSection(def);
                    if (ImGui::BeginCombo("##Экспозиция", report.GetSectionRangeStr(section).c_str()))
                    {
                        for (int i = 0; i < report.GetSectionCount(); ++i)
                        {
                            const bool isSelected = (section == i);
                            if (ImGui::Selectable(report.GetSectionRangeStr(i).c_str(), isSelected))
                            {
                                def.exposure = i;
                                rowChanged = true;
                            }

                            if (isSelected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }
                else
                {
                    if (ImGui::InputInt("##Координата", &def.coord, 1, 100))
                    {
                        def.coord = std::clamp(def.coord, 0, report.perimeter);
                        rowChanged = true;
                    }

                    ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(report.GetSectionRangeStr(report.GetDefectSection(def)).c_str());
                }

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Обозначение", report.GetDefectRTName(def.symbol).c_str()))
                {
                    for (int i = 0; i < static_cast<int>(DefectRtSymbol::Count); ++i)
                    {
                        auto symbol = static_cast<DefectRtSymbol>(i);
                        const bool isSelected = (def.symbol == symbol);
                        if (ImGui::Selectable(report.GetDefectRTName(symbol).c_str(), isSelected))
                        {
                            def.symbol = symbol;
                            rowChanged = true;
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                /// поля, которые у этого типа не печатаются, блокируются, но не стираются:
                /// при возврате к прежнему типу введённое вернётся
                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::BeginDisabled(!fields.length);
                rowChanged |= DefectSizeInput("##Протяжённость", def.length);
                ImGui::EndDisabled();

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::BeginDisabled(!fields.size);
                rowChanged |= DefectSizeInput("##Длина", def.width);
                ImGui::EndDisabled();

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::BeginDisabled(!fields.size);
                rowChanged |= DefectSizeInput("##Ширина", def.height);
                ImGui::EndDisabled();

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                ImGui::BeginDisabled(!fields.sign);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Окончание", def.endGreaterThan ? ">" : "≤"))
                {
                    if (ImGui::Selectable("≤", !def.endGreaterThan))
                    {
                        def.endGreaterThan = false;
                        rowChanged = true;
                    }
                    if (ImGui::Selectable(">", def.endGreaterThan))
                    {
                        def.endGreaterThan = true;
                        rowChanged = true;
                    }

                    ImGui::EndCombo();
                }
                ImGui::EndDisabled();

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                rowChanged |= ImGui::Checkbox("##Допустим", &def.acceptable);

                ImGui::TableNextColumn(); /////////////////////////////////////////////////////////////////
                if (ImGui::SmallButton("Удалить"))
                    removeRow = row; /// удаляем после цикла: def - ссылка на элемент вектора

                if (rowChanged)
                {
                    def.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                    changed = true;
                }

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        if (removeRow >= 0)
        {
            report.defRGCList.erase(report.defRGCList.begin() + removeRow);
            changed = true;
        }

        if (ImGui::Button("Добавить"))
        {
            changed = true;
            DefectRt def;
            if (!report.defRGCList.empty())
            {
                def.coord = report.defRGCList.back().coord;
                def.exposure = report.defRGCList.back().exposure;
            }
            report.defRGCList.push_back(def);
        }

        ImGui::SeparatorText("Как будет в заключении");

        /// примечаний ровно столько, сколько участков: при смене диаметра лишние отбрасываются
        const int sectionCount = report.GetSectionCount();
        if (report.sectionNotes.size() != static_cast<size_t>(sectionCount))
        {
            report.sectionNotes.resize(static_cast<size_t>(sectionCount));
            changed = true;
        }

        if (ImGui::BeginTable("Defect preview", 4, ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn(report.GetSectionTitle().c_str(), ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Описание дефектов");
            ImGui::TableSetupColumn("Заключение", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Примечание");
            ImGui::TableHeadersRow();

            for (int section = 0; section < sectionCount; ++section)
            {
                ImGui::TableNextRow();
                ImGui::PushID(section);

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(report.GetSectionRangeStr(section).c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextWrapped("%s", report.GetSectionDefectsStr(section).c_str());

                ImGui::TableSetColumnIndex(2);
                if (report.IsSectionAcceptable(section))
                    ImGui::TextUnformatted("Допустим");
                else
                    ImGui::TextColored(ImVec4(1.f, 0.3f, 0.3f, 1.f), "Не допустим");

                ImGui::TableSetColumnIndex(3);
                ImGui::SetNextItemWidth(-FLT_MIN);
                changed |= ImGui::InputText("##Примечание", &report.sectionNotes.at(static_cast<size_t>(section)));

                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        ImGui::SeparatorText("Заключение о годности сварного соединения");
        for (int i = 0; i < static_cast<int>(ControlResult::Count); ++i)
        {
            const auto item = static_cast<ControlResult>(i);

            if (i > 0)
                ImGui::SameLine(); /// вариантов четыре - помещаются в одну строку
            if (ImGui::RadioButton(GetControlResultStr(item).c_str(), report.controlResult == item))
            {
                report.controlResult = item;
                changed = true;
            }
        }

        if (changed)
            report.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
