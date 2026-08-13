#include "settingsWindow.hpp"
#include "imgui.h"
#include "applicationData.hpp"
#include "gui.hpp"

SettingsWindow::SettingsWindow(ApplicationData& coreAppData)
	: appData{ coreAppData }
{}

void SettingsWindow::Show(bool& isOpen)
{
	//ImGuiViewport* viewport = ImGui::GetMainViewport();
	//ImGui::SetNextWindowPos(viewport->Pos);

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse;

	if (ImGui::Begin("Settings", &isOpen, window_flags))
	{
		GetAppInformation();

		SetVsyncMode();

		ImGui::Spacing();

		SetGuiStyle();

		ImGui::Spacing();

		ImGui::SeparatorText("Font");
		if (ImGui::SliderFloat("##FontSize#", &appData.fontSize, 6.0f, appData.fontSizeMax, "Font size: %.0f"))
			ImGui::GetStyle().FontScaleMain = appData.fontSize / appData.fontSizeMax;

		SetFont();

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
	ImGuiIO& io = ImGui::GetIO();

	ImGui::SeparatorText("Info");
	ImGui::Text("API: %s", appData.rendererName.c_str());
	ImGui::Text("Driver: %s", appData.driverName.c_str());

	static float fpsUpdateTimer{ 0.f };
	static float currentFrametime{ 0.f };
	static float framerate{ 0.f };

	fpsUpdateTimer += io.DeltaTime;
	if (fpsUpdateTimer >= 0.5f)
	{
		if (io.Framerate > 0)
			currentFrametime = { 1000.f / io.Framerate };

		framerate = { io.Framerate };
		fpsUpdateTimer = { 0.f };
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
	if (ImGui::SliderInt("##Vsync#", &vsyncMode, 0, 1, appData.vsyncModeName.c_str()))
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
		default:
			SDL_SetRenderVSync(appData.renderer, SDL_RENDERER_VSYNC_DISABLED);
			appData.vsyncModeName = "Vsync: disabled";
			break;
		}
	}
}

void SettingsWindow::SetFont()
{
	static constexpr const char *fontNames[] = {
		"NotoSans Regular",
		"NotoSans Bold",
		"NotoSans Italic",
		"NotoSans Bold Italic",
		"ShareTechMonoRegular" };

	int currentFontIndex = static_cast<int>(appData.font);
	if (ImGui::Combo("##Font#", &currentFontIndex, fontNames, IM_ARRAYSIZE(fontNames)))
	{
		appData.font = static_cast<AppFont>(currentFontIndex);
		switch (appData.font)
		{
		case AppFont::ShareTechMonoRegular:
			ImGui::GetIO().FontDefault = appData.fontShareTechMonoRegular;
			break;
		case AppFont::NotoSansRegular:
			ImGui::GetIO().FontDefault = appData.fontNotoSansRegular;
			break;
		case AppFont::NotoSansBold:
			ImGui::GetIO().FontDefault = appData.fontNotoSansBold;
			break;
		case AppFont::NotoSansItalic:
			ImGui::GetIO().FontDefault = appData.fontNotoSansItalic;
			break;
		case AppFont::NotoSansBoldItalic:
			ImGui::GetIO().FontDefault = appData.fontNotoSansBoldItalic;
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
	ImGui::SameLine();
	ImGui::RadioButton("Custom", &currentStyleIndex, static_cast<int>(GuiStyle::Custom));

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
	
	case GuiStyle::Custom:
		Gui::SetCustomTheme();
		break;
	}
}