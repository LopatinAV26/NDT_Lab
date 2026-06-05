#include "nomogram.hpp"

#include "imgui.h"
#include "implot.h"

#include "arina9.hpp"
#include "eresco32.hpp"
#include "eresco65.hpp"

Nomogram::Nomogram()
{
    xray.reserve(3);
    xray.push_back(std::make_unique<Eresco32>());
    xray.push_back(std::make_unique<Eresco65>());
    xray.push_back(std::make_unique<Arina9>());
}

void Nomogram::NomogramWindow(bool &isOpen)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags = //ImGuiWindowFlags_NoDecoration |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Расчёт экспозиции", &isOpen, window_flags))
    {
        for (size_t i = 0; i < xray.size(); ++i)
        {
            plotTitle = xray.at(xrayTube_index)->GetData().xrayTubeName;
            mAminimum = xray.at(xrayTube_index)->GetData().mAminimum;
            mAmaximum = xray.at(xrayTube_index)->GetData().mAmaximum;
            currentAdjustment = xray.at(xrayTube_index)->GetData().currentAdjustment;
            xrayInfo = xray.at(xrayTube_index)->GetData().info;

            if (i % 2 == 1)
                ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::RadioButton(xray.at(i)->GetData().xrayTubeName.c_str(), &xrayTube_index, i);
            ImGui::PopID();
        }

        ImGui::Separator();
        ImGui::Text("%s", xrayInfo.c_str());

        ImGui::DragInt("Фокусное расстояние, мм", &focusDistance, 1, 10, 1400);
        ImGui::DragFloat("Толщина стали, мм", &materialThickness, 0.1f, 1.0f, 70.0f, "%.1f");
        // ImGui::SliderFloat("Требуемая плотность снимка, е.о.п.", &opticalDensity, 1.0f, 4.0f, "%.1f");

        if (!currentAdjustment && measurementUnits_index == 0)
            measurementUnits_index = 1;

        ImGui::BeginDisabled(!currentAdjustment);
        ImGui::RadioButton("мА х мин", &measurementUnits_index, 0);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::RadioButton("минуты", &measurementUnits_index, 1);
        ImGui::SameLine();
        ImGui::RadioButton("секунды", &measurementUnits_index, 2);

        bool mAIsDisabled{false};
        switch (measurementUnits_index)
        {
        case 0:
            nameAxisY = "Экспозиция, мА х мин";
            mAIsDisabled = true;
            break;
        case 1:
            nameAxisY = "Время экспозиции, мин";
            break;
        case 2:
            nameAxisY = "Время экспозиции, c";
            break;
        }

        if (mA < mAminimum || mA > mAmaximum)
            mA = mAminimum;

        ImGui::BeginDisabled(mAIsDisabled || !currentAdjustment);
        ImGui::SliderFloat("Сила тока, мА", &mA, mAminimum, mAmaximum, "%.1f");
        ImGui::EndDisabled();

        ImGui::SliderFloat("Множитель экспозиции", &exposureMultiplier, 0.5, 3.0, "%.1f");
        ImGui::SetItemTooltip("Подбирается индивидуально при тестовом просвечивании");

        if (ImPlot::BeginPlot(plotTitle.c_str(), ImVec2(-1, ImGui::GetContentRegionAvail().y - 90), ImPlotFlags_NoLegend))
        {
            ImPlot::SetupAxes("Толщина стали, мм", nameAxisY.c_str(), ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
            DrawNomogram(xrayTube_index);
            ImPlot::EndPlot();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::TextWrapped("Необходимо понимать, что рентгеновские плёнки и, особенно экраны имеют разброс по чувствительности, а рентгеновские аппараты - неизбежный технологический разброс параметров, поэтому рассчитанное по номограммам время не является точным, а служит только для предварительной оценки. При расчёте номограмм используются усреднённые паспортные значения чувствительности плёнок и экранов, а также линейных коэффициентов ослабления рентгеновского излучения в материалах. В каждом конкретном случае сочетание параметров может быть иным и время экспозиции может несколько отличаться от рассчитанного по номограммам. Также на итоговую оптическую плотность рентгеновского снимка влияет тип/марка проявителя, его концентрация, температура и время проявки.");
        ImGui::PopStyleColor();

        ImGui::End();
    }
}

void Nomogram::DrawNomogram(int xrayTube_index)
{
    for (size_t i = 0; i < xray.size(); ++i)
    {
        xray.at(xrayTube_index)->Draw(focusDistance, mA, measurementUnits_index, exposureMultiplier, materialThickness);
    }
}
