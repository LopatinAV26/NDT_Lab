#include "normativeDocumentsEditWindow.hpp"

#include <array>
#include <utility>
#include <fstream>
#include <filesystem>

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "methodsNdt.hpp"

void SDLCALL NormativeDocumentsEditWindow::OnFileSelected(void *userdata, const char *const *filelist, int /*filter*/)
{
    auto *self = static_cast<NormativeDocumentsEditWindow *>(userdata);

    if (filelist && filelist[0])
        self->pendingFilePath = filelist[0];

    self->fileDialogResultReady = true;
}

void NormativeDocumentsEditWindow::Show(NormativeDocument &normativeDocument, bool &isOpen)
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
            SDL_ShowOpenFileDialog(&NormativeDocumentsEditWindow::OnFileSelected, this, nullptr, nullptr, 0, nullptr, false);

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
