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

	/// @brief Вызывается из Core::ProcessEvent на SDL_EVENT_QUIT (крестик окна/Alt+F4) - вместо
	/// немедленного выхода показывает окно подтверждения на следующем кадре
	void RequestQuit() { quitRequested = true; }

	/// @brief true, когда пользователь подтвердил закрытие приложения в попапе - Core::Iterate
	/// проверяет это после кадра, чтобы вернуть SDL_APP_SUCCESS
	bool ShouldQuit() const { return quitConfirmed; }

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

	bool quitRequested = false; ///< триггер: открыть попап подтверждения на следующем кадре
	bool quitConfirmed = false; ///< пользователь подтвердил закрытие приложения
};
