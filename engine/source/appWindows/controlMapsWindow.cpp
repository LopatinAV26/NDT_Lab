#include "controlMapsWindow.hpp"

#include <array>
#include <utility>
#include <cfloat>
#include <format>
#include <fstream>
#include <filesystem>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "methodsNdt.hpp"

/// категории трубопровода и соответствующие им флаги техкарты - и для таблицы, и для формы редактирования
static constexpr std::array<std::pair<Category, bool ControlMap::*>, static_cast<size_t>(Category::Count)> categoryOptions{{
    {Category::H, &ControlMap::categoryB},
    {Category::I, &ControlMap::categoryI},
    {Category::II, &ControlMap::categoryII},
    {Category::III, &ControlMap::categoryIII},
    {Category::IV, &ControlMap::categoryIV},
}};

void ControlMapsWindow::Show(std::vector<ControlMap> &controlMapsList)
{
    static int tableRows = 0;     ///< количество строк в таблице
    static int editingIndex = -1; ///< текущий индекс техкарты, которая создаётся/редактируется

    if (ImGui::BeginTable("Технологические карты", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg, ImVec2(0, ImGui::GetContentRegionAvail().y - 50)))
    {
        ImGui::TableSetupColumn("Шифр");
        ImGui::TableSetupColumn("Метод\nконтроля");
        ImGui::TableSetupColumn("Диаметр");
        ImGui::TableSetupColumn("Толщина\nстенки");
        ImGui::TableSetupColumn("Категория\nтрубопровода");
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
                        controlMapsList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
                    ImGui::EndPopup();
                }

                ImGui::SameLine();
                NDT::TextWithTooltipIfTruncated(controlMapsList.at(row).code);

                ImGui::TableNextColumn();
                {
                    static constexpr std::array<std::pair<Method, bool ControlMap::*>, 8> methodOptions{{
                        {Method::VT, &ControlMap::forVT},
                        {Method::UT, &ControlMap::forUT},
                        {Method::RT, &ControlMap::forRT},
                        {Method::DRT, &ControlMap::forDRT},
                        {Method::PT, &ControlMap::forPT},
                        {Method::MT, &ControlMap::forMT},
                        {Method::LT, &ControlMap::forLT},
                        {Method::DT, &ControlMap::forDT},
                    }};

                    const ControlMap &controlMap = controlMapsList.at(row);
                    for (const auto &[method, member] : methodOptions)
                        if (controlMap.*member)
                        {
                            NDT::TextWithTooltipIfTruncated(GetMethodAbbreviation(method));
                            break;
                        }
                }

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(std::format("{:d}", controlMapsList.at(row).diameter));

                ImGui::TableNextColumn();
                NDT::TextWithTooltipIfTruncated(std::format("{:.1f}", controlMapsList.at(row).nominalWallThickness));

                ImGui::TableNextColumn();
                {
                    const ControlMap &controlMap = controlMapsList.at(row);
                    std::string categories;
                    for (const auto &[category, member] : categoryOptions)
                    {
                        if (!(controlMap.*member))
                            continue;
                        if (!categories.empty())
                            categories += ", ";
                        categories += GetCategoryStr(category);
                    }
                    NDT::TextWithTooltipIfTruncated(categories);
                }

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
        editWindow = true;
        editingIndex = tableRows - 1;
    }

    if (editWindow && editingIndex >= 0 && editingIndex < static_cast<int>(controlMapsList.size()))
        Edit(controlMapsList.at(editingIndex), editWindow);
}

void ControlMapsWindow::Edit(ControlMap &controlMap, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Технологическая карта", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Шифр техкарты");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##code#", &controlMap.code);

        ImGui::TextDisabled("Метод контроля");
        static constexpr std::array<std::pair<Method, bool ControlMap::*>, 8> methodOptions{{
            {Method::VT, &ControlMap::forVT},
            {Method::UT, &ControlMap::forUT},
            {Method::RT, &ControlMap::forRT},
            {Method::DRT, &ControlMap::forDRT},
            {Method::PT, &ControlMap::forPT},
            {Method::MT, &ControlMap::forMT},
            {Method::LT, &ControlMap::forLT},
            {Method::DT, &ControlMap::forDT},
        }};

        bool ControlMap::*selectedMethod = nullptr;
        for (size_t i = 0; i < methodOptions.size(); ++i)
        {
            const auto &[method, member] = methodOptions[i];
            if (ImGui::RadioButton(GetMethodAbbreviation(method).c_str(), controlMap.*member))
                selectedMethod = member;
            if (i + 1 < methodOptions.size())
                ImGui::SameLine();
        }
        if (selectedMethod)
        {
            for (const auto &[method, member] : methodOptions)
                controlMap.*member = (member == selectedMethod);
            changed = true;
        }

        ImGui::TextDisabled("Диаметр");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::DragInt("##diameter#", &controlMap.diameter, 1, 10, 1500, "Наружный диаметр трубы %d, мм");

        ImGui::TextDisabled("Толщина стенки");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::DragFloat("##nominalWallThickness#", &controlMap.nominalWallThickness, 0.1f, 1.0f, 50.0f, "Номинальная толщина стенки %.1f, мм");

        ImGui::TextDisabled("Категория трубопровода");
        for (size_t i = 0; i < categoryOptions.size(); ++i)
        {
            const auto &[category, member] = categoryOptions[i];
            changed |= ImGui::Checkbox(GetCategoryStr(category).c_str(), &(controlMap.*member));
            if (i + 1 < categoryOptions.size())
                ImGui::SameLine();
        }

        ImGui::TextDisabled("Описание");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputTextMultiline("##description#", &controlMap.description, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));

        ImGui::TextDisabled("Файл техкарты");
        if (controlMap.fileName.empty())
            ImGui::TextUnformatted("(не прикреплён)");
        else if (ImGui::TextLink(controlMap.fileName.c_str()))
            NDT::OpenFileFromBytes(controlMap.fileName, controlMap.fileData);

        if (ImGui::Button("Прикрепить файл..."))
            SDL_ShowOpenFileDialog(&ControlMapsWindow::OnFileSelected, this, nullptr, nullptr, 0, nullptr, false);

        ImGui::BeginDisabled(controlMap.fileName.empty());
        ImGui::SameLine();
        if (ImGui::Button("Открепить"))
        {
            controlMap.fileName.clear();
            controlMap.fileData.clear();
            changed = true;
        }
        ImGui::EndDisabled();

        if (fileDialogResultReady)
        {
            fileDialogResultReady = false;

            if (!pendingFilePath.empty())
            {
                std::filesystem::path filePath = NDT::PathFromUtf8(pendingFilePath);
                std::ifstream file(filePath, std::ios::binary);
                if (file)
                {
                    controlMap.fileData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
                    controlMap.fileName = NDT::PathToUtf8(filePath.filename());
                    changed = true;
                }

                pendingFilePath.clear();
            }
        }

        if (changed)
            controlMap.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}

void SDLCALL ControlMapsWindow::OnFileSelected(void *userdata, const char *const *filelist, int /*filter*/)
{
    auto *self = static_cast<ControlMapsWindow *>(userdata);

    if (filelist && filelist[0])
        self->pendingFilePath = filelist[0];

    self->fileDialogResultReady = true;
}
