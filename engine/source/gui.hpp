#pragma once

#include <memory>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_stdlib.h"
#include "implot.h"
#include "appWindows.hpp"

class Gui
{
public:
	explicit Gui(ApplicationData &coreAppData);
	~Gui();

	void InitImGui();
	void ProcessEventImGui(const SDL_Event *event);
	void IterateImGui();
	void RenderImGui();

private:
	void ButtonsWindow();

	ApplicationData &appData;

	SettingsWindow settingsWindow{appData};
	NomogramWindow nomogramWindow{appData};


	bool showButtonsWindow{true};
	bool showSettingsWindow{false};
	bool showNomogramWindow{false};
};
