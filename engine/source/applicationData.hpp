#pragma once

#include <string>
#include <SDL3/SDL.h>
#include <toml++/toml.hpp>
#include <filesystem>
//#include <fstream>

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

	std::string_view pathToDevices{"resources/xray_devices.toml"};

	SDL_Window *window{nullptr};
	SDL_Renderer *renderer{nullptr};
	int windowWidth{1920};
	int windowHeight{1080};
	SDL_WindowFlags windowFlags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY};
	const char *driverName{nullptr};
	float mainScale{1.0f};
	float windowRounding{0.0f};
	float frameRounding{0.0f};
	float grabRounding{0.0f};
	float fontSize{10.0f};
	GuiStyle style{2};

	bool isWindowFocused{true};
	bool isWindowMinimized{false};

private:
	toml::table tbl;
	const std::filesystem::path pathToSettings{"resources/settings.toml"};

	void LoadSettings()
	{
		try
		{
			tbl = toml::parse_file(pathToSettings.string());
			SDL_Log("Config file '%s' parsed successfully.\n", pathToSettings.c_str());
		}
		catch (const toml::parse_error &err)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR,
						 "Failed to parse config file '%s': %s\n",
						 pathToSettings.c_str(), err.what());
			return;
		}

		mainScale = tbl["mainScale"].value_or(mainScale);
		windowRounding = tbl["windowRounding"].value_or(windowRounding);
		frameRounding = tbl["frameRounding"].value_or(frameRounding);
		grabRounding = tbl["grabRounding"].value_or(grabRounding);
		fontSize = tbl["fontSize"].value_or(fontSize);
		style = tbl["guiStyle"].value_or(style);
	}

	void SaveSettings()
	{
		tbl.insert_or_assign("mainScale", mainScale);
		tbl.insert_or_assign("windowRounding", windowRounding);
		tbl.insert_or_assign("frameRounding", frameRounding);
		tbl.insert_or_assign("grabRounding", grabRounding);
		tbl.insert_or_assign("fontSize", fontSize);
		tbl.insert_or_assign("guiStyle", style);
		std::ofstream file(pathToSettings);
		if (!file)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR,
						 "Failed to open '%s' for writing",
						 pathToSettings.c_str());
			return;
		}
		file << tbl;
		SDL_Log("Settings saved to '%s'", pathToSettings.c_str());
	}
};
