#include "labWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "ImGuiDatePicker.hpp"
#include "applicationData.hpp"
#include "resourceManager.hpp"
#include "utilities.hpp"

LabWindow::LabWindow(ApplicationData &coreAppData, ResourceManager &resourceManager)
    : lab{coreAppData},
      nomogramWindow{coreAppData, resourceManager}
{
}

void LabWindow::Show(bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Лаборатория НК", &isOpen, window_flags))
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
            ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            ImGui::OpenPopup("Закрыть окно?");

        if (ImGui::BeginPopupModal("Закрыть окно?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            ImGui::Text("Выйти из лаборатории?");
            ImGui::Separator();

            if (ImGui::Button("Да", ImVec2(120, 0)))
            {
                lab.SaveDB();
                isOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Нет", ImVec2(120, 0)))
                ImGui::CloseCurrentPopup();

            ImGui::SetItemDefaultFocus();

            ImGui::EndPopup();
        }

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Главная"))
            {
                ShowMain();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Параметры контроля"))
            {
                nomogramWindow.Show();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Отчёты контроля"))
            {
                reportWindow.Show(lab.reportsList, lab);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Сотрудники"))
            {
                emplWindow.Show(lab.employeesList);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Сотрудники надзора"))
            {
                otherEmplWindow.Show(lab.inspectorsList);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void LabWindow::ShowMain()
{
    const float availWidth = ImGui::GetContentRegionAvail().x;
    bool changed = false;

    auto centeredText = [availWidth](const std::string &text)
    {
        float textWidth = ImGui::CalcTextSize(text.c_str()).x;
        ImGui::SetCursorPosX((availWidth - textWidth) * 0.5f);
        ImGui::TextUnformatted(text.c_str());
    };

    if (!editingLabInfo)
    {
        centeredText(lab.labInfo.laboratoryName);
        centeredText(lab.labInfo.numberAttestation);
        centeredText(NDT::FormatDateForDisplay(lab.labInfo.attestationEndDate));
    }
    else
    {
        ImGui::TextDisabled("Наименование лаборатории");
        //ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##Наименование лаборатории#", &lab.labInfo.laboratoryName);
        ImGui::TextDisabled("Номер свидетельства об аттестации");
        //ImGui::SetNextItemWidth(-FLT_MIN);
        changed |= ImGui::InputText("##Номер свидетельства об аттестации#", &lab.labInfo.numberAttestation);

        ImGui::TextDisabled("Дата окончания действия аттестации");
        tm date = NDT::ParseIsoDateTm(lab.labInfo.attestationEndDate);
        //ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::DatePicker("##Дата окончания действия аттестации#", date))
        {
            lab.labInfo.attestationEndDate = NDT::FormatIsoDateTm(date);
            changed = true;
        }
    }

    if (changed)
        lab.labInfo.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());

    ImGui::Spacing();
    const char *linkLabel = editingLabInfo ? "Готово" : "Изменить";
    float linkWidth = ImGui::CalcTextSize(linkLabel).x;
    ImGui::SetCursorPosX((availWidth - linkWidth) * 0.5f);
    if (ImGui::TextLink(linkLabel))
        editingLabInfo = !editingLabInfo;
}
