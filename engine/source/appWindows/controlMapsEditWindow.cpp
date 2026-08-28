#include "controlMapsEditWindow.hpp"

#include <fstream>
#include <filesystem>

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"

void SDLCALL ControlMapsEditWindow::OnFileSelected(void *userdata, const char *const *filelist, int /*filter*/)
{
    auto *self = static_cast<ControlMapsEditWindow *>(userdata);

    if (filelist && filelist[0])
        self->pendingFilePath = filelist[0];

    self->fileDialogResultReady = true;
}

void ControlMapsEditWindow::Show(ControlMap &controlMap, bool &isOpen)
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
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##method#", &controlMap.method);

        ImGui::TextDisabled("Диаметр");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##diameter#", &controlMap.diameter);

        ImGui::TextDisabled("Толщина стенки");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##thickness#", &controlMap.thickness);

        ImGui::TextDisabled("Описание");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputTextMultiline("##description#", &controlMap.description, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));

        ImGui::TextDisabled("Файл техкарты");
        if (controlMap.fileName.empty())
            ImGui::TextUnformatted("(не прикреплён)");
        else if (ImGui::TextLink(controlMap.fileName.c_str()))
            NDT::OpenFileFromBytes(controlMap.fileName, controlMap.fileData);

        if (ImGui::Button("Прикрепить файл..."))
            SDL_ShowOpenFileDialog(&ControlMapsEditWindow::OnFileSelected, this, nullptr, nullptr, 0, nullptr, false);

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
