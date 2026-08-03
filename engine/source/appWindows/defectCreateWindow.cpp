#include "defectCreateWindow.hpp"

#include <cfloat>
#include "laboratory.hpp"

void DefectCreateWindow::ShowDefectCreateWindow(Report &report, bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    if (ImGui::Begin("Конструктор дефектов", &isOpen, window_flags))
    {
        int tableRows = 0;

        if (ImGui::BeginTable("Defect creator", 7, ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Координата");
            ImGui::TableSetupColumn("Обозначение");
            ImGui::TableSetupColumn("Протяжённость,\nдля Ac и Ab");
            ImGui::TableSetupColumn("Длина");
            ImGui::TableSetupColumn("Ширина");
            ImGui::TableSetupColumn("Превышение\nплотности");
            ImGui::TableSetupColumn("Запись дефекта");
            ImGui::TableHeadersRow();

            tableRows = static_cast<int>(report.defRGCList.size());
            for (int row = 0; row < tableRows; ++row)
            {
                DefRT &def = report.defRGCList.at(row);
                ImGui::TableNextRow();
                ImGui::PushID(row);

                ImGui::TableSetColumnIndex(0); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputInt("##Координата#", &def.coord, 1, 100);
                if (def.coord < 0)
                    def.coord = 0;

                ImGui::TableSetColumnIndex(1); //////////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Обозначение#", def.name[def.nameIndex].c_str()))
                {
                    for (int i = 0; i < std::ssize(def.name); ++i)
                    {
                        const bool isSelected = (def.nameIndex == i);
                        if (ImGui::Selectable(def.name[i].c_str(), isSelected))
                            def.nameIndex = i;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                bool length = (def.name[def.nameIndex] == "Ac" || def.name[def.nameIndex] == "Ab") ? false : true;
                ImGui::BeginDisabled(length);
                ImGui::TableSetColumnIndex(2); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat("##Протяжённость#", &def.length, 0.1f, 1.0f, "%.1f");
                if (def.length < 0.0f)
                    def.length = 0.0f;
                ImGui::EndDisabled();

                ImGui::TableSetColumnIndex(3); //////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat("##Длина#", &def.width, 0.1f, 1.0f, "%.1f");
                if (def.width < 0.0f)
                    def.width = 0.0f;

                ImGui::TableSetColumnIndex(4); //////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat("##Ширина#", &def.height, 0.1f, 1.0f, "%.1f");
                if (def.height < 0.0f)
                    def.height = 0.0f;

                ImGui::TableSetColumnIndex(5); //////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Окончание#", def.end[def.endIndex].c_str()))
                {
                    for (int i = 0; i < std::ssize(def.end); ++i)
                    {
                        const bool isSelected = (def.endIndex == i);
                        if (ImGui::Selectable(def.end[i].c_str(), isSelected))
                            def.endIndex = i;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ConstructDefectRGCString(def);

                ImGui::TableSetColumnIndex(6); //////////////////////////////
                if (tableRows > 0)
                {
                    // ImGui::Text("(%d) %s", data.defectList[row].coord, data.defectList[row].record.c_str());
                    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
                    ImGui::TextUnformatted(std::format("({:d}) {}", def.coord, def.record).c_str()); // безопаснее
                    ImGui::PopTextWrapPos();

                    if (ImGui::Button("-"))
                    {
                        tableRows--;
                        report.defRGCList.resize(tableRows);
                    }
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        if (ImGui::Button("+"))
        {
            tableRows++;
            report.defRGCList.resize(tableRows);
        }
    }
    ImGui::End();
}

void DefectCreateWindow::ConstructDefectRGCString(DefRT &input)
{
    // Ас 25.0 – 2.0 × 1.0 ≤	пример записи дефекта
    // A   B   C  D  E  F  G
    std::string A = input.name.at(input.nameIndex);
    std::string B;
    std::string C = "-";
    std::string D = std::format("{:.1f}", input.width);
    std::string E = "×";
    std::string F = std::format("{:.1f}", input.height);
    std::string G = input.end.at(input.endIndex);

    if (A == "Ac" || A == "Ab")
        B = std::format("{:.1f}", input.length);
    else
    {
        B = "";
        C = "";
    }

    input.coordStr = std::format("({:d}) ", input.coord);
    input.record = A + " " + B + C + D + E + F + G;
}