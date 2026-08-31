#include "equipmentWindow.hpp"

#include <array>
#include <utility>
#include <string>
#include <fstream>
#include <filesystem>
#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "ImGuiDatePicker.hpp"
#include "methodsNdt.hpp"

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
        editWindow = true;
        editingIndex = tableRows - 1;
    }
    if (editWindow && editingIndex >= 0 &&
        editingIndex < static_cast<int>(equipmentList.size()))
        Edit(equipmentList.at(editingIndex), editWindow);
}

void EquipmentWindow::Edit(Equipment &equipment, bool &isOpen)
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
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::VT).c_str(), &equipment.forVT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::UT).c_str(), &equipment.forUT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::RT).c_str(), &equipment.forRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::DRT).c_str(), &equipment.forDRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::PT).c_str(), &equipment.forPT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::MT).c_str(), &equipment.forMT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::LT).c_str(), &equipment.forLT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::ECT).c_str(), &equipment.forECT);

        std::array<std::pair<bool, std::string>, 8> methodFlags = {{
            {equipment.forVT, GetMethodAbbreviation(Method::VT)},
            {equipment.forUT, GetMethodAbbreviation(Method::UT)},
            {equipment.forRT, GetMethodAbbreviation(Method::RT)},
            {equipment.forDRT, GetMethodAbbreviation(Method::DRT)},
            {equipment.forPT, GetMethodAbbreviation(Method::PT)},
            {equipment.forMT, GetMethodAbbreviation(Method::MT)},
            {equipment.forLT, GetMethodAbbreviation(Method::LT)},
            {equipment.forECT, GetMethodAbbreviation(Method::ECT)},
        }};

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

        changed |= ImGui::Checkbox("Поверяется/калибруется", &equipment.isCalibrated);

        ImGui::BeginDisabled(!equipment.isCalibrated);

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

        ImGui::TextDisabled("Файл свидетельства");
        if (equipment.fileName.empty())
            ImGui::TextUnformatted("(не прикреплён)");
        else if (ImGui::TextLink(equipment.fileName.c_str()))
            NDT::OpenFileFromBytes(equipment.fileName, equipment.fileData);

        if (ImGui::Button("Прикрепить файл..."))
            SDL_ShowOpenFileDialog(&EquipmentWindow::OnFileSelected, this, nullptr, nullptr, 0, nullptr, false);

        ImGui::BeginDisabled(equipment.fileName.empty());
        ImGui::SameLine();
        if (ImGui::Button("Открепить"))
        {
            equipment.fileName.clear();
            equipment.fileData.clear();
            changed = true;
        }
        ImGui::EndDisabled();

        ImGui::EndDisabled(); // !equipment.isCalibrated

        if (fileDialogResultReady)
        {
            fileDialogResultReady = false;

            if (!pendingFilePath.empty())
            {
                std::filesystem::path filePath = NDT::PathFromUtf8(pendingFilePath);
                std::ifstream file(filePath, std::ios::binary);
                if (file)
                {
                    equipment.fileData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
                    equipment.fileName = NDT::PathToUtf8(filePath.filename());
                    changed = true;
                }

                pendingFilePath.clear();
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

void SDLCALL EquipmentWindow::OnFileSelected(void *userdata, const char *const *filelist, int /*filter*/)
{
    auto *self = static_cast<EquipmentWindow *>(userdata);

    if (filelist && filelist[0])
        self->pendingFilePath = filelist[0];

    self->fileDialogResultReady = true;
}
