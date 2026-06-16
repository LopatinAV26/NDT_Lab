#pragma once

#include <SDL3/SDL.h>
#include <toml++/toml.hpp>
#include <filesystem>

enum class GuiStyle
{
	Dark,
	Light,
	Classic
};

class ApplicationData
{
public:
	ApplicationData();
	~ApplicationData();

	SDL_Window* window{ nullptr };
	SDL_Renderer* renderer{ nullptr };
	SDL_WindowFlags windowFlags{ SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
	float mainScale{ 1.0f };
	float windowRounding{ 0.0f };
	float frameRounding{ 0.0f };
	float grabRounding{ 0.0f };
	float fontSize{ 10.0f };
	int windowWidth{ 1920 };
	int windowHeight{ 1080 };
	GuiStyle style{ 2 };
	std::filesystem::path pathToDevices = "resources/xray_devices.toml";
	const char* driverName{ nullptr };
	bool isWindowFocused{ true };
	bool isWindowMinimized{ false };

private:
	void LoadSettings();
	void SaveSettings();

	std::filesystem::path pathToSettings = "resources/settings.toml";
	toml::table tbl;
};
