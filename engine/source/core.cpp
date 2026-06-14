#include "core.hpp"

SDL_AppResult Core::Init()
{
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	appData.window = SDL_CreateWindow("NDT Lab",
									   appData.windowWidth,
									   appData.windowHeight,
									   appData.windowFlags);

	appData.mainScale = SDL_GetWindowDisplayScale(appData.window);
	if (appData.mainScale <= 0.0f)
		appData.mainScale = 1.0f;

	if (!appData.window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	appData.renderer = SDL_CreateRenderer(appData.window, NULL); // openGL; vulkan; direct3d12
	if (!appData.renderer)
	{
		SDL_DestroyWindow(appData.window);
		appData.window = nullptr;
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_Log("SDL initialized successfully.");
	appData.driverName = SDL_GetCurrentVideoDriver();

	//SDL_SetRenderVSync(appData.renderer, SDL_RENDERER_VSYNC_ADAPTIVE);

	imWindow = std::make_unique<Gui>(appData);
	imWindow->InitImGui();

	return SDL_APP_CONTINUE;
}

SDL_AppResult Core::Iterate()
{
	if (appData.isWindowMinimized)
	{
		SDL_Delay(100);
		return SDL_APP_CONTINUE;
	}

	if (!appData.isWindowFocused)
	{
		SDL_Delay(66);
	}

	imWindow->IterateImGui();
	SDL_SetRenderDrawColor(appData.renderer, 64, 64, 64, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(appData.renderer);

	// Update();
	// Render();

	imWindow->RenderImGui();
	SDL_RenderPresent(appData.renderer);

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
		appData.isWindowMinimized = true;
		break;

	case SDL_EVENT_WINDOW_RESTORED:
		appData.isWindowMinimized = false;
		break;

	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		appData.isWindowFocused = true;
		break;

	case SDL_EVENT_WINDOW_FOCUS_LOST:
		appData.isWindowFocused = false;
		break;

	case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		appData.mainScale = SDL_GetWindowDisplayScale(appData.window);
		if (appData.mainScale <= 0.0f)
			appData.mainScale = 1.0f;
		break;
	}

	return SDL_APP_CONTINUE;
}

Core::~Core()
{
	imWindow.reset();
	SDL_DestroyRenderer(appData.renderer);
	SDL_DestroyWindow(appData.window);
	SDL_Quit();

	SDL_Log("SDL shutdown complete.");
}