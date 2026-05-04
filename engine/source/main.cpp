#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include "core.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
	Core *core = new Core();
	*appstate = core;

	return core->Init();
};

SDL_AppResult SDL_AppIterate(void *appstate)
{
	Core *core = static_cast<Core *>(appstate);
	return core->Iterate();
};

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	Core *core = static_cast<Core *>(appstate);
	return core->ProcessEvent(event);
};

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	Core *core = static_cast<Core *>(appstate);
	delete core;
};