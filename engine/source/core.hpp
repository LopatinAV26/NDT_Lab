#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>

#include "applicationData.hpp"
#include "gui.hpp"

class Core
{
public:
	Core() = default;
	Core(const Core &) = delete;
	Core &operator=(const Core &) = delete;
	Core(Core &&) = delete;
	Core &operator=(Core &&) = delete;
	~Core();

	SDL_AppResult Init();
	SDL_AppResult Iterate();
	SDL_AppResult ProcessEvent(const SDL_Event *event);

private:
	ApplicationData appData;
	std::unique_ptr<Gui> imWindow;
};
