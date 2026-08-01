#pragma once

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_stdlib.h"
#include "appWindows/settingsWindow.hpp"
#include "appWindows/labWindow.hpp"

class ApplicationData;
class ResourceManager;

class Gui
{
public:
	explicit Gui(ApplicationData &coreAppData, ResourceManager &resourceManager);
	~Gui();

	void InitImGui();
	void ProcessEventImGui(const SDL_Event *event);
	void IterateImGui();
	void RenderImGui();

private:
	void ButtonsWindow();
	void SetRadiationTheme();

	ApplicationData &appData;
	ResourceManager &resManager;

	SettingsWindow settingsWindow;
	LabWindow labWindow;

	bool buttonsWindowOpen = true;
	bool settingsWindowOpen = false;
	bool labWindowIsOpen = false;
};
