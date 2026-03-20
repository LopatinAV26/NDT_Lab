#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include <memory>

#include "core.hpp"

auto core = std::make_unique<Core>();

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
	return core->Init();
};

SDL_AppResult SDL_AppIterate(void* appstate)
{
	return core->Iterate();
};

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
	return core->ProcessEvent(event);
};

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
};