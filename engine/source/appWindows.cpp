#include "appWindows.hpp"
#include <cmath>
#include <algorithm>

SettingsWindow::SettingsWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
{
}

void SettingsWindow::Show(bool &isOpen)
{
	ImGuiIO &io = ImGui::GetIO();
	if (ImGui::Begin("Settings", &isOpen))
	{
		ImGui::SeparatorText("Info");
		ImGui::Text("API: %s", SDL_GetRendererName(appData.renderer));
		ImGui::Text("Driver: %s", appData.driverName.c_str() ? appData.driverName.c_str() : "unknown");

		static float fpsUpdateTimer{0.f};
		static float currentFrametime{0.f};
		static float framerate{0.f};

		fpsUpdateTimer += io.DeltaTime;
		if (fpsUpdateTimer >= 0.5f)
		{
			if (io.Framerate > 0)
				currentFrametime = {1000.f / io.Framerate};

			framerate = {io.Framerate};
			fpsUpdateTimer = {0.f};
		}
		ImGui::Text("Application average %.2f ms/frame (%.0f FPS)", currentFrametime, framerate);

		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse pos: <INVALID>");

		ImGui::Spacing();
		ImGui::SeparatorText("GUI Style");
		SetGuiStyle();

		ImGui::Spacing();
		ImGui::SeparatorText("Rounding");
		if (ImGui::SliderFloat("WindowRounding", &appData.windowRounding, 0.f, 12.f, "%.0f"))
			ImGui::GetStyle().WindowRounding = appData.windowRounding;

		if (ImGui::SliderFloat("FrameRounding", &appData.frameRounding, 0.f, 12.f, "%.0f"))
			ImGui::GetStyle().FrameRounding = appData.frameRounding;

		if (ImGui::SliderFloat("GrabRounding", &appData.grabRounding, 0.f, 12.f, "%.0f"))
			ImGui::GetStyle().GrabRounding = appData.grabRounding;
	}
	ImGui::End();
}

void SettingsWindow::SetGuiStyle() const
{
	int currentStyleIndex = static_cast<int>(appData.style);

	ImGui::RadioButton("Dark", &currentStyleIndex, static_cast<int>(GuiStyle::Dark));
	ImGui::SameLine();
	ImGui::RadioButton("Light", &currentStyleIndex, static_cast<int>(GuiStyle::Light));
	ImGui::SameLine();
	ImGui::RadioButton("Classic", &currentStyleIndex, static_cast<int>(GuiStyle::Classic));

	if (currentStyleIndex == static_cast<int>(appData.style))
		return;

	appData.style = static_cast<GuiStyle>(currentStyleIndex);

	switch (appData.style)
	{
	case GuiStyle::Dark:
		ImGui::StyleColorsDark();
		break;

	case GuiStyle::Light:
		ImGui::StyleColorsLight();
		break;

	case GuiStyle::Classic:
		ImGui::StyleColorsClassic();
		break;
	}
}
/////////////////////////////////////////////
NomogramWindow::NomogramWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
{
	devices = NDT::LoadDevices(appData.pathToDevices);
	calculatedDevices = devices;
}

void NomogramWindow::Show(bool &isOpen)
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
	// ImGuiWindowFlags_NoTitleBar |
	// ImGuiWindowFlags_NoMove |
	// ImGuiWindowFlags_NoResize |
	// ImGuiWindowFlags_NoCollapse |
	// ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Диаграмма экспозиции", &isOpen, window_flags))
	{
#if defined(_WIN32)
		ImGui::BeginChild("Header", ImVec2(0, 30 * appData.mainScale));
#elif defined(__linux__)
		ImGui::BeginChild("Header", ImVec2(0, 30));
#else
		ImGui::BeginChild("Header", ImVec2(0, 30));
#endif

		ImGui::Text("Диаграмма экспозиции");

#if defined(_WIN32)
		ImGui::SameLine(ImGui::GetWindowWidth() - 60 * appData.mainScale);
#elif defined(__linux__)
		ImGui::SameLine(ImGui::GetWindowWidth() - 60);
#else
		ImGui::SameLine(ImGui::GetWindowWidth() - 60);
#endif

		if (ImGui::Button("Выход"))
			isOpen = false;

		ImGui::EndChild();

		if (devices.empty() || calculatedDevices.empty())
		{
			ImGui::Text("No devices loaded.");
			ImGui::End();
			return;
		}

		if (ImGui::BeginCombo("Аппарат", calculatedDevices[deviceIndex].name.c_str()))
		{
			for (int i = 0; i < std::ssize(calculatedDevices); ++i)
			{
				const bool isSelected = (deviceIndex == i);
				if (ImGui::Selectable(calculatedDevices[i].name.c_str(), isSelected))
					deviceIndex = i;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			if (deviceCurrent < calculatedDevices[deviceIndex].currentMinimum ||
				deviceCurrent > calculatedDevices[deviceIndex].currentMaximum)
				deviceCurrent = calculatedDevices[deviceIndex].currentMinimum;

			ImGui::EndCombo();
		}
		ImGui::SameLine();

		ImGui::Button("?");
		ImGui::SetItemTooltip("%s", calculatedDevices[deviceIndex].information.c_str());

		ImGui::DragFloat("##Фокусное расстояние#", &focusDistance, 1.f, 1.f, 2000.f, "Фокусное расстояние: %.0f мм");

		if (steelThickness > steelThicknessMax)
			steelThickness = steelThicknessMax;

		if (steelThickness < steelThicknessMin)
			steelThickness = steelThicknessMin;

		ImGui::DragFloat("##Толщина стали#",
						 &steelThickness,
						 0.1f,
						 steelThicknessMin,
						 steelThicknessMax,
						 "Толщина стали: %.1f мм");

		ImGui::BeginDisabled(!calculatedDevices[deviceIndex].currentAdjustment ||
							 measureIndex == Measure::mAxmin);
		ImGui::SliderFloat("##Сила тока#",
						   &deviceCurrent,
						   calculatedDevices[deviceIndex].currentMinimum,
						   calculatedDevices[deviceIndex].currentMaximum,
						   "Сила тока: %.1f мА");
		ImGui::EndDisabled();

		if (!calculatedDevices[deviceIndex].currentAdjustment && measureIndex == Measure::mAxmin)
		{
			measureIndex = Measure::min;
			nameAxisY = "Время экспозиции, мин";
		}

		ImGui::BeginDisabled(!calculatedDevices[deviceIndex].currentAdjustment);
		if (ImGui::RadioButton("мА х мин", measureIndex == Measure::mAxmin))
		{
			measureIndex = Measure::mAxmin;
			nameAxisY = "Экспозиция, мА х мин";
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::RadioButton("минуты", measureIndex == Measure::min))
		{
			measureIndex = Measure::min;
			nameAxisY = "Время экспозиции, мин";
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("секунды", measureIndex == Measure::sec))
		{
			measureIndex = Measure::sec;
			nameAxisY = "Время экспозиции, с";
		}

		if (ImPlot::BeginPlot(calculatedDevices[deviceIndex].name.c_str(),
							  ImVec2(-1, ImGui::GetContentRegionAvail().y),
							  ImPlotFlags_NoLegend))
		{
			ImPlot::SetupAxes("Толщина стали, мм", nameAxisY.c_str(),
							  ImPlotAxisFlags_AutoFit,
							  ImPlotAxisFlags_AutoFit);

			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

			calculatedDevices = ExposureRecalculation(devices, focusDistance, deviceCurrent);

			std::vector<CurvesRef> visibleLines; // массив структур массивов
			visibleLines.reserve(calculatedDevices[deviceIndex].curveVector.size());

			for (const auto &curve : calculatedDevices[deviceIndex].curveVector)
			{
				if (calculatedDevices[deviceIndex].electricPower >= curve.voltage * deviceCurrent)
				{
					ImPlot::PlotLine(curve.label.c_str(),
									 curve.xData.data(),
									 curve.yData.data(),
									 static_cast<int>(curve.xData.size()));

					visibleLines.push_back({curve.xData, curve.yData,
											ImPlot::GetLastItemColor(), curve.label});
				}
			}
			if (!visibleLines.empty())
			{
				steelThicknessMin = visibleLines.front().x.front();
				steelThicknessMax = visibleLines.back().x.back();
			}
			DrawMarkers(visibleLines, steelThickness);

			ImPlot::EndPlot();
		}
	}
	ImGui::End();
}

std::vector<NDT::XrayDevice> NomogramWindow::ExposureRecalculation(const std::vector<NDT::XrayDevice> &deviceVector,
																   float distance, float current) const
{
	auto result = deviceVector;
	auto &device = result[deviceIndex];

	float factor = std::pow((distance / deviceVector[deviceIndex].focusDistanceDefault), 2);

	if (!result[deviceIndex].currentAdjustment)
		current = 1.f;

	switch (measureIndex)
	{
	case Measure::min:
		factor /= current; // мА х мин переводим в минуты
		break;
	case Measure::sec:
		factor /= current / 60.f; // переводим в секунды
		break;
	default:
		break;
	}

	for (auto &curve : device.curveVector)
	{
		for (auto &y : curve.yData)
		{
			y *= factor;
		}
	}

	return result;
}

// AI generated
void NomogramWindow::DrawMarkers(const std::vector<CurvesRef> &curves, float thickness) const
{
	if (curves.empty())
		return;
	if (!std::isfinite(thickness))
		return;

	ImPlotSpec lineSpec;
	lineSpec.LineColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

	ImPlot::PlotInfLines("##thickness-line", &thickness, 1, lineSpec);

	for (const auto &line : curves)
	{
		if (line.x.empty() || line.y.empty())
			continue;

		const auto &xs = line.x;
		const auto &ys = line.y;

		if (xs.size() < 2 || xs.size() != ys.size())
			continue;

		for (size_t i = 0; i + 1 < xs.size(); ++i)
		{
			const float x0 = xs.at(i);
			const float x1 = xs.at(i + 1);
			const float minX = std::min(x0, x1);
			const float maxX = std::max(x0, x1);

			if (thickness < minX || thickness > maxX)
				continue;

			const float delta = x1 - x0;
			if (std::fabs(delta) <= std::numeric_limits<float>::epsilon())
				continue;

			const float t = std::clamp((thickness - x0) / delta, 0.0f, 1.0f);

			double px = {thickness};

			const float y0 = ys.at(i);
			const float y1 = ys.at(i + 1);
			// интерполяция по логарифму для логарифмической шкалы
			double py = std::pow(10.0, std::log10(y0) + t * (std::log10(y1) - std::log10(y0)));

			const ImVec4 fillColor = line.color;
			const ImVec4 outlineColor{0.05f, 0.05f, 0.05f, 1.0f};

			std::string markerLabel = "##thickness-marker-" + line.label + "-" + std::to_string(i);

			ImPlotSpec markerSpec;
			markerSpec.LineWeight = 2.0f;
			markerSpec.Marker = ImPlotMarker_Diamond;
			markerSpec.MarkerSize = 6.0f;
			markerSpec.MarkerLineColor = outlineColor;
			markerSpec.MarkerFillColor = fillColor;
			ImPlot::PlotScatter(markerLabel.c_str(), &px, &py, 1, markerSpec);

			switch (measureIndex)
			{
			case Measure::mAxmin:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f  T=%.1f %s", line.label.c_str(), px, py, "мА х мин");
				break;
			case Measure::min:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f  T=%.1f %s", line.label.c_str(), px, py, "мин");
				break;
			case Measure::sec:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f  T=%.0f %s", line.label.c_str(), px, py, "с");
				break;
			default:
				ImPlot::Annotation(px, py, fillColor, ImVec2(8.0f, -14.0f), true,
								   "%s S=%.1f  T=%.1f", line.label.c_str(), px, py);
				break;
			}

			break;
		}
	}
}
