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
                reportWindow.Show(lab.reportList);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Сотрудники"))
            {
                Employees(lab.employeesList);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void LabWindow::Employees(std::vector<Employee> &empl)
{
    static int tableRows = 0;
    if (ImGui::BeginTable("Сотрудники", 15,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Имя");
        ImGui::TableSetupColumn("Организация");
        ImGui::TableSetupColumn("Подразделение");
        ImGui::TableSetupColumn("Должность");
        ImGui::TableSetupColumn("Дата\nтрудоустройства");
        ImGui::TableSetupColumn("Опыт работы");
        ImGui::TableSetupColumn("Личный код");
        ImGui::TableSetupColumn("Разряд");
        ImGui::TableSetupColumn("Номер\nудостоверения");
        ImGui::TableSetupColumn("");
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        tableRows = empl.size();
        for (int row = 0; row < tableRows; ++row)
        {
            ImGui::TableNextRow();
            ImGui::PushID(row);

            bool changed = false;

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##name#", &empl.at(row).name);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputTextMultiline("##organization#", &empl.at(row).organization);
            ImGui::SetItemTooltip("%s", empl.at(row).organization.c_str());

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##department#", &empl.at(row).department);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##position#", &empl.at(row).position);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##employeementDate#", &empl.at(row).employeementDate);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##experience#", &empl.at(row).experience);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##personalCode#", &empl.at(row).personalCode);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##level#", &empl.at(row).level);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##certificateNumber#", &empl.at(row).certificateNumber);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            if (changed)
            {
                empl.at(row).updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
            }

            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("+")) //////////////////////////////////////////
    {
        tableRows++;
        empl.resize(tableRows);
    }
}
