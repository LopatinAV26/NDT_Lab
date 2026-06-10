#include "appWindows.hpp"
#include <cmath>

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
		ImGui::Text("Driver: %s", appData.driverName ? appData.driverName : "unknown");

		fpsUpdateTimer += io.DeltaTime;
		if (fpsUpdateTimer >= 0.5f)
		{
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

		ImGui::End();
	}
}

void SettingsWindow::SetGuiStyle()
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
		ImGui::BeginChild("Header", ImVec2(0, 30));

		ImGui::Text("Диаграмма экспозиции");

		ImGui::SameLine(
			ImGui::GetWindowWidth() - 60);

		if (ImGui::Button("Выход"))
			isOpen = false;

		ImGui::EndChild();

		if (devices.empty() || calculatedDevices.empty())
		{
			ImGui::Text("No devices loaded.");
			ImGui::End();
			return;
		}

		if (ImGui::BeginCombo("Аппарат", devices[deviceIndex].name.c_str()))
		{
			for (int i = 0; i < std::ssize(devices); ++i)
			{
				const bool isSelected = (deviceIndex == i);
				if (ImGui::Selectable(devices[i].name.c_str(), isSelected))
					deviceIndex = i;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			if (current < devices[deviceIndex].currentMinimum ||
				current > devices[deviceIndex].currentMaximum)
				current = devices[deviceIndex].currentMinimum;

			if (steelThickness > devices[deviceIndex].steelThicknessMax)
				steelThickness = 10.f;

			ImGui::EndCombo();
		}

		ImGui::Text("%s", devices[deviceIndex].information.c_str());

		ImGui::DragInt("Фокусное расстояние", &focusDistance, 1, 0, 2000);
		calculatedDevices = ExposureRecalculation(devices, focusDistance);

		ImGui::DragFloat("Толщина стали, мм",
						 &steelThickness, 0.1f, 0.5f,
						 devices[deviceIndex].steelThicknessMax, "%.1f");

		ImGui::BeginDisabled(!devices[deviceIndex].currentAdjustment ||
							 measurementUnits_index == 0);
		ImGui::SliderFloat("Сила тока",
						   &current,
						   devices[deviceIndex].currentMinimum,
						   devices[deviceIndex].currentMaximum, "%.1f");
		ImGui::EndDisabled();

		if (!devices[deviceIndex].currentAdjustment &&
			measurementUnits_index == 0)
			measurementUnits_index = 1;

		ImGui::BeginDisabled(!devices[deviceIndex].currentAdjustment);
		ImGui::RadioButton("мА х мин", &measurementUnits_index, 0);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::RadioButton("минуты", &measurementUnits_index, 1);
		ImGui::SameLine();
		ImGui::RadioButton("секунды", &measurementUnits_index, 2);

		const char *nameAxisY = "Экспозиция";
		switch (measurementUnits_index)
		{
		case 0:
			nameAxisY = "Экспозиция, мА х мин";
			break;
		case 1:
			nameAxisY = "Время экспозиции, мин";
			break;
		case 2:
			nameAxisY = "Время экспозиции, с";
			break;
		default:
			break;
		}

		if (ImPlot::BeginPlot(devices[deviceIndex].name.c_str(),
							  ImVec2(-1, ImGui::GetContentRegionAvail().y),
							  ImPlotFlags_NoLegend))
		{
			ImPlot::SetupAxes("Толщина стали, мм", nameAxisY,
							  ImPlotAxisFlags_AutoFit,
							  ImPlotAxisFlags_AutoFit);

			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
			for (const auto &curve : calculatedDevices[deviceIndex].curveVector)
			{
				ImPlot::PlotLine(curve.label.c_str(),
								 curve.xData.data(),
								 curve.yData.data(),
								 curve.xData.size());
			}
			ImPlot::EndPlot();
		}
	}
	ImGui::End();
}

std::vector<NDT::XrayDevice> NomogramWindow::ExposureRecalculation(const std::vector<NDT::XrayDevice> &deviceVector,
																   int distance)
{
	auto result = deviceVector;
	auto &device = result[deviceIndex];

	float factor = std::pow((static_cast<float>(deviceVector[deviceIndex].focusDistanceDefault) /
							 static_cast<float>(distance)),
							2);

	switch (measurementUnits_index)
	{
	case 1:
		factor /= current; // мА х мин переводим в минуты
		break;
	case 2:
		factor *= 60.f / current; // переводим в секунды
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
