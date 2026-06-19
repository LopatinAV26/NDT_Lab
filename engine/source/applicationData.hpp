#pragma once

#include <SDL3/SDL.h>
#include <filesystem>
#include <string>

enum class GuiStyle
{
	Dark,
	Light,
	Classic
};

enum class Vsync
{
	adaptive = -1,
	disabled,
	enabled
};

class ApplicationData
{
public:
	ApplicationData();
	~ApplicationData();

	SDL_Window *window = {nullptr};
	SDL_Renderer *renderer = {nullptr};
	SDL_WindowFlags windowFlags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY};
	std::filesystem::path pathToDevices = "resources/xray_devices.toml";
	std::string driverName;
	std::string rendererName;
	GuiStyle style = GuiStyle::Classic;
	Vsync mode = Vsync::enabled;
	std::string vsyncModeName;
	float mainScale = {1.0f};
	float windowRounding = {0.0f};
	float frameRounding = {0.0f};
	float grabRounding = {0.0f};
	float fontSize = {10.0f};
	int windowWidth = {1920};
	int windowHeight = {1080};
	bool isWindowFocused = {true};
	bool isWindowMinimized = {false};

private:
	void LoadSettings(const std::filesystem::path &path = "resources/settings.toml");
	void SaveSettings(const std::filesystem::path &path = "resources/settings.toml");
};
