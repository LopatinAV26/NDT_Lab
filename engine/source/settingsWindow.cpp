#include "settingsWindow.hpp"
#include "imgui.h"
//#include "imgui_stdlib.h"
#include "applicationData.hpp"

SettingsWindow::SettingsWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
{
}

void SettingsWindow::Show(bool &isOpen)
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);

	if (ImGui::Begin("Settings", &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		GetAppInformation();

		SetVsyncMode();

		ImGui::Spacing();

		SetGuiStyle();

		ImGui::Spacing();

		ImGui::SeparatorText("Rounding");
		if (ImGui::SliderFloat("##WindowRounding#", &appData.windowRounding, 0.f, 12.f, "Window rounding: %.0f"))
			ImGui::GetStyle().WindowRounding = appData.windowRounding;

		if (ImGui::SliderFloat("##FrameRounding#", &appData.frameRounding, 0.f, 12.f, "Frame rounding: %.0f"))
			ImGui::GetStyle().FrameRounding = appData.frameRounding;

		if (ImGui::SliderFloat("##GrabRounding#", &appData.grabRounding, 0.f, 12.f, "Grab rounding: %.0f"))
			ImGui::GetStyle().GrabRounding = appData.grabRounding;
	}
	ImGui::End();
}

void SettingsWindow::GetAppInformation() const
{
	ImGuiIO &io = ImGui::GetIO();

	ImGui::SeparatorText("Info");
	ImGui::Text("API: %s", appData.rendererName.c_str());
	ImGui::Text("Driver: %s", appData.driverName.c_str());

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
}

void SettingsWindow::SetVsyncMode()
{
	int vsyncMode = static_cast<int>(appData.mode);
	if (ImGui::SliderInt("##Vsync#", &vsyncMode, -1, 1, appData.vsyncModeName.c_str()))
	{
		appData.mode = static_cast<Vsync>(vsyncMode);
		switch (appData.mode)
		{
		case Vsync::disabled:
			if (!SDL_SetRenderVSync(appData.renderer, SDL_RENDERER_VSYNC_DISABLED))
				SDL_Log("Failed to set VSync: %s", SDL_GetError());
			appData.vsyncModeName = "Vsync: disabled";
			break;
		case Vsync::enabled:
			if (!SDL_SetRenderVSync(appData.renderer, 1))
			{
				SDL_Log("Failed to set VSync: %s", SDL_GetError());
				appData.mode = Vsync::disabled;
			}
			appData.vsyncModeName = "Vsync: enabled";
			break;
		case Vsync::adaptive:
			if (!SDL_SetRenderVSync(appData.renderer, SDL_RENDERER_VSYNC_ADAPTIVE))
			{
				SDL_Log("Failed to set VSync: %s", SDL_GetError());
				appData.mode = Vsync::disabled;
			}
			appData.vsyncModeName = "Vsync: adaptive";
			break;
		default:
			SDL_SetRenderVSync(appData.renderer, SDL_RENDERER_VSYNC_DISABLED);
			appData.vsyncModeName = "Vsync: disabled";
			break;
		}
	}
}

void SettingsWindow::SetGuiStyle()
{
	ImGui::SeparatorText("GUI Style");
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