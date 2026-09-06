#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include "resourceManager.hpp"
#include "applicationData.hpp"
#include "gui.hpp"

class Laboratory;

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
	std::unique_ptr<ResourceManager>resManager;
	std::unique_ptr<Laboratory> lab; ///< модель данных живёт здесь, а не в окне - окна получают её по ссылке
	std::unique_ptr<Gui> imWindow;
};
