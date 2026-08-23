#include "equipmentEditWindow.hpp"

#include <array>
#include <utility>
#include <string>

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"

void EquipmentEditWindow::Show(Equipment &equipment, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Оборудование", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Наименование оборудования");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##name#", &equipment.name);

        ImGui::TextDisabled("Метод контроля");
        changed |= ImGui::Checkbox("ВИК", &equipment.forVT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("УК", &equipment.forUT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("РК", &equipment.forRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("ЦРК", &equipment.forDRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("ПВК", &equipment.forPT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("МПК", &equipment.forMT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("ПВТ", &equipment.forLT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("ВК", &equipment.forECT);

        std::array<std::pair<bool, std::string>, 8> methodFlags = {{{equipment.forVT, "ВИК"}, {equipment.forUT, "УК"}, {equipment.forRT, "РК"}, {equipment.forDRT, "ЦРК"}, {equipment.forPT, "ПВК"}, {equipment.forMT, "МПК"}, {equipment.forLT, "ПВТ"}, {equipment.forECT, "ВК"}}};

        equipment.method.clear();
        for (const auto &[isSet, label] : methodFlags)
        {
            if (!isSet)
                continue;

            if (!equipment.method.empty())
                equipment.method += ", ";
            equipment.method += label;
        }

        ImGui::TextDisabled("Назначение");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##function#", &equipment.function);

        ImGui::TextDisabled("Производитель");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##manufacturer#", &equipment.manufacturer);

        ImGui::TextDisabled("Серийный номер");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##serialNumber#", &equipment.serialNumber);

        ImGui::TextDisabled("Год выпуска");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##yearOfManufacture#", &equipment.yearOfManufacture);

        ImGui::TextDisabled("Год ввода в эксплуатацию");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##yearOfCommissioning#", &equipment.yearOfCommissioning);

        ImGui::TextDisabled("Технические и метрологические характеристики");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##technicalAndMetrologicalCharacteristics#", &equipment.technicalAndMetrologicalCharacteristics);

        ImGui::TextDisabled("Владелец");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##owner#", &equipment.owner);

        ImGui::TextDisabled("Номер документа о поверке/калибровке");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##certificateNumber#", &equipment.certificateNumber);

        ImGui::TextDisabled("Дата документа о поверке/калибровке");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(equipment.certificateDate);
            if (ImGui::DatePicker("##certificateDate#", date))
            {
                equipment.certificateDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Дата окончания действия документа о поверке/калибровке");
        ImGui::SetNextItemWidth(-FLT_MIN);
        {
            tm date = NDT::ParseIsoDateTm(equipment.certificateEndDate);
            if (ImGui::DatePicker("##certificateEndDate#", date))
            {
                equipment.certificateEndDate = NDT::FormatIsoDateTm(date);
                changed = true;
            }
        }

        ImGui::TextDisabled("Состояние оборудования");
        std::array<std::pair<bool *, std::string>, 5> stateFlags = {{{&equipment.isOperational, "Исправно"},
                                                                      {&equipment.isUnderRepair, "В ремонте"},
                                                                      {&equipment.isFaulty, "Неисправно"},
                                                                      {&equipment.isPendingDisposal, "Подлежит списанию"},
                                                                      {&equipment.isPreserved, "Законсервировано"}}};

        for (size_t i = 0; i < stateFlags.size(); ++i)
        {
            if (ImGui::RadioButton(stateFlags[i].second.c_str(), *stateFlags[i].first))
            {
                for (auto &[flag, label] : stateFlags)
                    *flag = false;
                *stateFlags[i].first = true;
                changed = true;
            }

            if (i + 1 < stateFlags.size())
                ImGui::SameLine();
        }

        equipment.state.clear();
        for (const auto &[flag, label] : stateFlags)
        {
            if (*flag)
            {
                equipment.state = label;
                break;
            }
        }

        if (changed)
            equipment.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}
