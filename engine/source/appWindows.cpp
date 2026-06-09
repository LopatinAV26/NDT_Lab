#include "appWindows.hpp"

SettingsWindow::SettingsWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
{
}

void SettingsWindow::Show(bool &isOpen)
{
	ImGuiIO &io = ImGui::GetIO();
	ImGui::Begin("Settings", &isOpen);
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
	}
	ImGui::End();
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
NomogramWindow::NomogramWindow()
{
	devices = manager.LoadDevices();
	calculatedDevices = devices;
}

void NomogramWindow::Show(bool &isOpen)
{
	/* ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags = // ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings; */

	if (ImGui::Begin("Расчёт экспозиции", &isOpen /*, window_flags*/))
	{
		if (devices.empty() && calculatedDevices.empty())
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

			ImGui::EndCombo();
		}

		ImGui::Text("%s", devices[deviceIndex].information.c_str());

		if (ImGui::DragInt("Фокусное расстояние", &focusDistance, 1, 0, 2000))
			calculatedDevices = ExposureRecalculation(devices, focusDistance);

		ImGui::DragFloat("Толщина стали, мм", &steelThickness, 0.1f, 0.5f, devices[deviceIndex].steelThicknessMax, "%.1f");

		if (ImPlot::BeginPlot("Диаграмма экспозиции РА",
							  ImVec2(-1, ImGui::GetContentRegionAvail().y - 100)
							  // ImPlotFlags_NoLegend
							  ))
		{
			ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
			for (int i = 0; i < std::ssize(calculatedDevices[deviceIndex].curveVector); ++i)
			{
				ImPlot::PlotLine(calculatedDevices[deviceIndex].curveVector[i].label.c_str(),
								 calculatedDevices[deviceIndex].curveVector[i].xData.data(),
								 calculatedDevices[deviceIndex].curveVector[i].yData.data(),
								 calculatedDevices[deviceIndex].curveVector[i].xData.size());
			}
			ImPlot::EndPlot();
		}
	}
	ImGui::End();
}

std::vector<XrayDevice> NomogramWindow::ExposureRecalculation(const std::vector<XrayDevice> &deviceVector,
													   int distance)
{
	std::vector<XrayDevice> result = deviceVector;
	for (int i = 0; i < std::ssize(result[deviceIndex].curveVector); ++i)
	{
		for (int j = 0; j < std::ssize(result[deviceIndex].curveVector[i].yData); ++j)
		{
			result[deviceIndex].curveVector[i].yData[j] /=
				std::pow((static_cast<float>(deviceVector[deviceIndex].focusDistanceDefault) /
						  static_cast<float>(distance)),
						 2);
		}
	}
	return result;
}
