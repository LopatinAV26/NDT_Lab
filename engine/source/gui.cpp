#include "gui.hpp"
#include <SDL3/SDL.h>
#include "embeddedFonts.hpp"
#include "applicationData.hpp"

Gui::Gui(ApplicationData &coreAppData)
	: appData{coreAppData}
{
}

void Gui::InitImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	[[maybe_unused]] ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	if (ImFont *font = LoadEmbeddedShareTechMono(io, (appData.fontSize * appData.mainScale)))
		io.FontDefault = font;

	switch (appData.style)
	{
	case GuiStyle::Dark:
		ImGui::StyleColorsDark();
		break;

	case GuiStyle::Light:
		ImGui::StyleColorsLight();
		break;

	case GuiStyle::Classic:
		ImGui::StyleColorsClassic();
		break;
	default:
		ImGui::StyleColorsClassic();
		break;
	}

	ImGuiStyle &style = ImGui::GetStyle();
#ifdef WIN32
	style.ScaleAllSizes(appData.mainScale);
#endif
	style.WindowRounding = appData.windowRounding;
	style.FrameRounding = appData.frameRounding;
	style.GrabRounding = appData.grabRounding;

	ImGui_ImplSDL3_InitForSDLRenderer(appData.window, appData.renderer);
	ImGui_ImplSDLRenderer3_Init(appData.renderer);

	SDL_Log("ImGui initialized successfully.");
}

void Gui::ProcessEventImGui(const SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
}

// Добавить в начало SDL_AppIterate
void Gui::IterateImGui()
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	//---------------------------------
	if (buttonsWindowOpen)
		ButtonsWindow();

	if (settingsWindowOpen)
		settingsWindow.Show(settingsWindowOpen);

	if (nomogramWindowOpen)
		nomogramWindow.Show(nomogramWindowOpen);

	if (protocolWindowOpen)
		protocolWindow.Show(protocolWindowOpen);
	//---------------------------------

	// Для работы с HiDPI
	ImGuiIO &io = ImGui::GetIO();
	SDL_SetRenderScale(appData.renderer,
					   io.DisplayFramebufferScale.x,
					   io.DisplayFramebufferScale.y);
}

// Добавить перед SDL_RenderPresent
void Gui::RenderImGui()
{
	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), appData.renderer);
}

void Gui::ButtonsWindow()
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetCenter());

	/*ImGuiWindowFlags window_flags = // ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus; */

	// Кнопки без окна
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
									// ImGuiWindowFlags_NoMove |
									ImGuiWindowFlags_AlwaysAutoResize |
									// ImGuiWindowFlags_NoSavedSettings |
									ImGuiWindowFlags_NoBringToFrontOnFocus |
									ImGuiWindowFlags_NoBackground;

	if (ImGui::Begin("Main Window", &buttonsWindowOpen, window_flags))
	{
		if (ImGui::Button("Настройки"))
			settingsWindowOpen = true;

		if (ImGui::Button("Номограмма"))
			nomogramWindowOpen = true;

		if (ImGui::Button("Протокол"))
			protocolWindowOpen = true;
	}
	ImGui::End();
}

Gui::~Gui()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
	SDL_Log("ImGui shutdown complete.");
}
