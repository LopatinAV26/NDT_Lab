#include "labWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "applicationData.hpp"
#include "resourceManager.hpp"

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
            if (ImGui::BeginTabItem("Параметры контроля"))
            {
                nomogramWindow.Show();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Отчёты контроля"))
            {
                reportWindow.ShowReportsWindow(lab.reportList, lab);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Сотрудники"))
            {
                emplWindow.ShowEmployeesWindow(lab.employeesList);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}