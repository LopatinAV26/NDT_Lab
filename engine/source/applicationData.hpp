#pragma once

#include <string>

#include <SDL3/SDL.h>

struct ApplicationData
{
	SDL_Window* window{ nullptr };
	SDL_Renderer* renderer{ nullptr };
	int windowWidth{ 1280 };
	int windowHeight{ 720 };
	SDL_WindowFlags windowFlags{ SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY };
	float mainScale{ 1.25f };
	const char* driverName{ nullptr };
	float windowRounding{ 4.0f };
	//const char *fontName{"resources/fonts/ShareTechMonoRegular.ttf"};
	float fontSize{ 13.0f };

	bool isWindowFocused{ true };
	bool isWindowMinimized{ false };
};
