#pragma once

#include <string>

#include <SDL3/SDL.h>

enum class GuiStyle
{
	Dark,
	Light,
	Classic
};

struct ApplicationData
{
	SDL_Window *window{nullptr};
	SDL_Renderer *renderer{nullptr};
	int windowWidth{1920};
	int windowHeight{1080};
	SDL_WindowFlags windowFlags{SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY};
	float mainScale{1.0f};
	const char *driverName{nullptr};
	float windowRounding{0.0f};
	float frameRounding{0.0f};
	float grabRounding{0.0f};
	float fontSize{13.0f};
	GuiStyle style{GuiStyle::Classic};

	bool isWindowFocused{true};
	bool isWindowMinimized{false};
};
