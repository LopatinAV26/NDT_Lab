#include "defectCreateWindow.hpp"

#include <cfloat>
#include <format>
#include "laboratory.hpp"

void DefectCreateWindow::Show(Report &report, bool &isOpen)
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
            ImGui::TableSetupColumn("Протяжённость");
            ImGui::TableSetupColumn("Длина");
            ImGui::TableSetupColumn("Ширина");
            ImGui::TableSetupColumn("Превышение\nплотности");
            ImGui::TableSetupColumn("Запись дефекта");
            ImGui::TableHeadersRow();

            tableRows = static_cast<int>(report.defRGCList.size());
            for (int row = 0; row < tableRows; ++row)
            {
                DefectRt &def = report.defRGCList.at(row);
                ImGui::TableNextRow();
                ImGui::PushID(row);

                ImGui::TableSetColumnIndex(0); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputInt("##Координата#", &def.coord, 1, 100))
                {
                    if (def.coord < 0)
                        def.coord = 0;
                    if (def.coord > report.perimeter)
                        def.coord = report.perimeter;
                }

                ImGui::TableSetColumnIndex(1); //////////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::BeginCombo("##Обозначение#", report.GetDefectRTName(def.symbol).c_str()))
                {
                    for (int i = 0; i < static_cast<int>(DefectRtSymbol::Count); ++i)
                    {
                        auto symbol = static_cast<DefectRtSymbol>(i);
                        const bool isSelected = (def.symbol == symbol);
                        if (ImGui::Selectable(report.GetDefectRTName(symbol).c_str(), isSelected))
                            def.symbol = symbol;

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                //bool length = (def.symbol == DefectRtSymbol::Ac || def.symbol == DefectRtSymbol::Ab) ? false : true;
               // ImGui::BeginDisabled(length);
                ImGui::TableSetColumnIndex(2); /////////////////////////////////////////////////////////////////
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputFloat("##Протяжённость#", &def.length, 0.1f, 1.0f, "%.1f");
                if (def.length < 0.0f)
                    def.length = 0.0f;
               //ImGui::EndDisabled();

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
                if (ImGui::BeginCombo("##Окончание#", def.endGreaterThan ? ">" : "≤"))
                {
                    if (ImGui::Selectable("≤", !def.endGreaterThan))
                        def.endGreaterThan = false;
                    if (ImGui::Selectable(">", def.endGreaterThan))
                        def.endGreaterThan = true;

                    ImGui::EndCombo();
                }

                ConstructDefectRT(report, def);

                ImGui::TableSetColumnIndex(6); //////////////////////////////
                if (tableRows > 0)
                {
                    ImGui::TextUnformatted(std::format("({:d}) {}", def.coord, def.record).c_str()); // безопаснее
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }

        if (ImGui::Button("Добавить"))
        {
            const int prevCoord = tableRows > 0 ? report.defRGCList.at(tableRows - 1).coord : 0;

            tableRows++;
            report.defRGCList.resize(tableRows);
            report.defRGCList.back().coord = prevCoord;
        }
    }
    ImGui::End();
}

void DefectCreateWindow::ConstructDefectRT(const Report &report, DefectRt &input)
{
    // Ас 25.0 – 2.0 × 1.0 ≤	пример записи дефекта
    // A   B   C  D  E  F  G
    std::string A = report.GetDefectRTName(input.symbol);
    std::string B = std::format("{:.1f}", input.length);
    std::string C = "-";
    std::string D = std::format("{:.1f}", input.width);
    std::string E = "×";
    std::string F = std::format("{:.1f}", input.height);
    std::string G = input.endGreaterThan ? ">" : "≤";

    input.resultLength = 0.f;

    switch (input.symbol)
    {
    case DefectRtSymbol::Ac:
    case DefectRtSymbol::Ab:
        input.record = A + B + C + D + E + F + G;
        input.resultLength = input.length;
        break;
    case DefectRtSymbol::E:
    case DefectRtSymbol::Mw:
        input.record = A + B;
        input.resultLength = input.length;
        break;
    case DefectRtSymbol::Fa:
    case DefectRtSymbol::Fb:
        input.record = A + B + G;
        input.resultLength = input.length;
        break;
    case DefectRtSymbol::Fe:
        input.record = A + B + G;
        break;
    case DefectRtSymbol::Fc1:
    case DefectRtSymbol::Fd:
    case DefectRtSymbol::delta1:
    case DefectRtSymbol::delta2:
        input.record = A;
        break;
    default:
        input.record = A + D + E + F + G;
        input.resultLength = input.width;
        break;
    }

    input.coordStr = std::format("({:d}) ", input.coord);
}