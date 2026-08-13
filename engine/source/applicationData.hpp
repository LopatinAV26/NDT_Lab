#pragma once

#include <SDL3/SDL.h>
#include "imgui.h"
#include <filesystem>
#include <string>

enum class GuiStyle
{
	Dark,
	Light,
	Classic,
	Custom
};

enum class AppFont
{
	NotoSansRegular,
	NotoSansBold,
	NotoSansItalic,
	NotoSansBoldItalic,
	ShareTechMonoRegular
};

enum class Vsync
{
	disabled,
	enabled
};

class ApplicationData
{
public:
	ApplicationData();
	~ApplicationData();

	SDL_Window *window = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	std::filesystem::path pathToDevices = "resources/xray_devices.toml";
	std::filesystem::path pathToDatabase = "resources/data.lnkdb";
	const char *pathToImGuiIniFile = "resources/imgui.ini";
	std::string driverName;
	std::string rendererName;
	GuiStyle style = GuiStyle::Classic;
	Vsync mode = Vsync::enabled;
	std::string vsyncModeName;
	float mainScale = 1.0f;
	float windowRounding = 0.0f;
	float frameRounding = 0.0f;
	float grabRounding = 0.0f;
	float fontSize = 10.0f;
	const float fontSizeMax = 32.0f; // размер, на котором печётся атлас шрифта; fontSize только уменьшает через FontGlobalScale
	AppFont font = AppFont::NotoSansRegular;

	// Заполняются в Gui::InitImGui после загрузки шрифтов в атлас
	//ImFont *fontShareTechMono = nullptr;
	ImFont *fontNotoSansRegular = nullptr;
	ImFont *fontNotoSansBold = nullptr;
	ImFont *fontNotoSansItalic = nullptr;
	ImFont *fontNotoSansBoldItalic = nullptr;
	ImFont* fontShareTechMonoRegular = nullptr;
	int windowWidth = 1920;
	int windowHeight = 1080;
	float windowScreenPercent = 0.8f; // доля от usable-области экрана
	bool isWindowFocused = true;
	bool isWindowMinimized = false;

private:
	void LoadSettings(const std::filesystem::path &path = "resources/settings.toml");
	void SaveSettings(const std::filesystem::path &path = "resources/settings.toml");
};
