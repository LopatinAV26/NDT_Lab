#include "controlMapsEditWindow.hpp"

#include <array>
#include <utility>
#include <fstream>
#include <filesystem>

#include "laboratory.hpp"
#include "utilities.hpp"
#include "imgui_stdlib.h"
#include "methodsNdt.hpp"

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
        changed |= ImGui::InputInt("##diameter#", &controlMap.diameter);

        ImGui::TextDisabled("Толщина стенки");
        ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputFloat("##thickness#", &controlMap.thickness, 0.0f, 0.0f, "%.1f");

        ImGui::TextDisabled("Категория трубопровода");
        changed |= ImGui::Checkbox("В", &controlMap.categoryB);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("I", &controlMap.categoryI);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("II", &controlMap.categoryII);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("III", &controlMap.categoryIII);
        ImGui::SameLine();
        changed |= ImGui::Checkbox("IV", &controlMap.categoryIV);

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
