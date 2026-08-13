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

	/// @brief Пересчитать масштаб (шрифт + отступы) от appData.mainScale/fontSize - вызывать при инициализации
	/// и при смене DPI монитора (SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
	void ApplyScale();
	static void SetCustomTheme();

private:
	void ButtonsWindow();
	

	ApplicationData &appData;
	ResourceManager &resManager;

	SettingsWindow settingsWindow;
	LabWindow labWindow;

	ImGuiStyle referenceStyle; ///< немасштабированный эталон стиля - ScaleAllSizes каждый раз считаем от него, не от текущего

	bool buttonsWindowOpen = true;
	bool settingsWindowOpen = false;
	bool labWindowIsOpen = false;
};
