#include "normativeDocumentsWindow.hpp"

#include <array>
#include <utility>
#include <fstream>
#include <filesystem>
#include <cfloat>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "methodsNdt.hpp"

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
            NDT::TextWithTooltipIfTruncated(std::format("{:d}", normativeDocumentsList.at(row).year));

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
        editWindow = true;
        editingIndex = tableRows - 1;
    }
    if (editWindow && editingIndex >= 0 &&
        editingIndex < static_cast<int>(normativeDocumentsList.size()))
        Edit(normativeDocumentsList.at(editingIndex), editWindow);
}

void NormativeDocumentsWindow::Edit(NormativeDocument &normativeDocument, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Нормативный документ", &isOpen, window_flags))
    {
        bool changed = false;

        ImGui::TextDisabled("Шифр");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##code#", &normativeDocument.code);

        ImGui::TextDisabled("Название документа");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputTextMultiline("##name#", &normativeDocument.name);

        ImGui::TextDisabled("Метод контроля");
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::VT).c_str(), &normativeDocument.forVT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::UT).c_str(), &normativeDocument.forUT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::RT).c_str(), &normativeDocument.forRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::DRT).c_str(), &normativeDocument.forDRT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::PT).c_str(), &normativeDocument.forPT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::MT).c_str(), &normativeDocument.forMT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::LT).c_str(), &normativeDocument.forLT);
        ImGui::SameLine();
        changed |= ImGui::Checkbox(GetMethodAbbreviation(Method::ECT).c_str(), &normativeDocument.forECT);

        std::array<std::pair<bool, std::string>, 8> methodFlags = {{
            {normativeDocument.forVT, GetMethodAbbreviation(Method::VT)},
            {normativeDocument.forUT, GetMethodAbbreviation(Method::UT)},
            {normativeDocument.forRT, GetMethodAbbreviation(Method::RT)},
            {normativeDocument.forDRT, GetMethodAbbreviation(Method::DRT)},
            {normativeDocument.forPT, GetMethodAbbreviation(Method::PT)},
            {normativeDocument.forMT, GetMethodAbbreviation(Method::MT)},
            {normativeDocument.forLT, GetMethodAbbreviation(Method::LT)},
            {normativeDocument.forECT, GetMethodAbbreviation(Method::ECT)},
        }};

        normativeDocument.method.clear();
        for (const auto &[isSet, label] : methodFlags)
        {
            if (!isSet)
                continue;

            if (!normativeDocument.method.empty())
                normativeDocument.method += ", ";
            normativeDocument.method += label;
        }

        ImGui::TextDisabled("Статус");
        static constexpr std::array<const char *, 3> statuses{"действующий", "отменён", "заменён"};
        for (const char *status : statuses)
        {
            if (ImGui::RadioButton(status, normativeDocument.status == status))
            {
                normativeDocument.status = status;
                changed = true;
            }
            ImGui::SameLine();
        }
        ImGui::NewLine();

        ImGui::TextDisabled("Год введения");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputInt("##year#", &normativeDocument.year);

        ImGui::TextDisabled("Файл документа");
        if (normativeDocument.fileName.empty())
            ImGui::TextUnformatted("(не прикреплён)");
        else if (ImGui::TextLink(normativeDocument.fileName.c_str()))
            NDT::OpenFileFromBytes(normativeDocument.fileName, normativeDocument.fileData);

        if (ImGui::Button("Прикрепить файл..."))
            SDL_ShowOpenFileDialog(&NormativeDocumentsWindow::OnFileSelected, this, nullptr, nullptr, 0, nullptr, false);

        ImGui::BeginDisabled(normativeDocument.fileName.empty());
        ImGui::SameLine();
        if (ImGui::Button("Открепить"))
        {
            normativeDocument.fileName.clear();
            normativeDocument.fileData.clear();
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
                    normativeDocument.fileData.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
                    normativeDocument.fileName = NDT::PathToUtf8(filePath.filename());
                    changed = true;
                }

                pendingFilePath.clear();
            }
        }

        if (changed)
            normativeDocument.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    }
    ImGui::End();
}

void SDLCALL NormativeDocumentsWindow::OnFileSelected(void *userdata, const char *const *filelist, int /*filter*/)
{
    auto *self = static_cast<NormativeDocumentsWindow *>(userdata);

    if (filelist && filelist[0])
        self->pendingFilePath = filelist[0];

    self->fileDialogResultReady = true;
}
