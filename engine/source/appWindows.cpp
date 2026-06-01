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
