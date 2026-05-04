#include "core.hpp"

SDL_AppResult Core::Init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	appData->mainScale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

	appData->window = SDL_CreateWindow("NDT",
									   (appData->windowWidth * appData->mainScale),
									   (appData->windowHeight * appData->mainScale),
									   appData->windowFlags);
	if (!appData->window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	appData->renderer = SDL_CreateRenderer(appData->window, NULL); // openGL; vulkan; direct3d12
	if (!appData->renderer)
	{
		SDL_DestroyWindow(appData->window);
		appData->window = nullptr;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Log("SDL initialized successfully.");
	appData->driverName = SDL_GetCurrentVideoDriver();

	// SDL_SetRenderVSync(coreData->renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

	imWindow = std::make_unique<Gui>(appData);
	imWindow->InitImGui();

	return SDL_APP_CONTINUE;
}

SDL_AppResult Core::Iterate()
{
	// Если окно свёрнуто или не в фокусе - приостанавливаем основной цикл
	if (appData->isWindowMinimized || !appData->isWindowFocused)
	{
		SDL_Delay(100); // Задержка 100мс для снижения нагрузки на CPU
		return SDL_APP_CONTINUE;
	}

	imWindow->IterateImGui();
	// SDL_SetRenderDrawColor(appData->renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(appData->renderer);

	// Update();
	// Render();

	imWindow->RenderImGui();
	SDL_RenderPresent(appData->renderer);

	return SDL_APP_CONTINUE;
}

SDL_AppResult Core::ProcessEvent(const SDL_Event *event)
{
	imWindow->ProcessEventImGui(event);

	switch (event->type)
	{
	case SDL_EVENT_QUIT:
		return SDL_APP_SUCCESS;

	case SDL_EVENT_WINDOW_MINIMIZED:
		appData->isWindowMinimized = true;
		break;

	case SDL_EVENT_WINDOW_RESTORED:
		appData->isWindowMinimized = false;
		break;

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		appData->isWindowFocused = true;
		break;

	case SDL_EVENT_WINDOW_FOCUS_LOST:
		appData->isWindowFocused = false;
		break;
	}

	return SDL_APP_CONTINUE;
}

Core::~Core()
{
	imWindow.reset();
	SDL_DestroyRenderer(appData->renderer);
	SDL_DestroyWindow(appData->window);
	SDL_Quit();

	SDL_Log("SDL shutdown complete.");
}