#pragma once

#include <memory>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_stdlib.h"

#include "implot.h"

#include "protocolVMC.hpp"
#include "nomogram.hpp"

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
	std::unique_ptr<ProtocolVMC> protocolVMC;
	std::unique_ptr<Nomogram> nomogram;

	bool showFullscreenWindow{true};
	bool showSettingsWindow{false};

	bool showProtocolVMC{false};
	bool showNomogram{false};

	float fpsUpdateTimer{0.f};
	float currentFrametime{0.f};
	float framerate{0.f};
};
