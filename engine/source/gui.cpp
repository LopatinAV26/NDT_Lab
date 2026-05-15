#include "gui.hpp"

#include <SDL3/SDL.h>

#include "embeddedFonts.hpp"

Gui::Gui(const std::shared_ptr<ApplicationData> appData)
	: appData{ appData }
{}

void Gui::InitImGui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();
	[[maybe_unused]] ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	/*io.Fonts->AddFontFromFileTTF(appData->fontName,
								 appData->fontSize,
								 &font_config);*/

	if (ImFont* font = LoadEmbeddedShareTechMono(io, appData->fontSize))
		io.FontDefault = font;

	// ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(appData->mainScale);
	style.FontScaleDpi = appData->mainScale;
	style.WindowRounding = appData->windowRounding;

	ImGui_ImplSDL3_InitForSDLRenderer(appData->window, appData->renderer);
	ImGui_ImplSDLRenderer3_Init(appData->renderer);

	SDL_Log("ImGui initialized successfully.");
}

void Gui::ProcessEventImGui(const SDL_Event* event)
{
	ImGui_ImplSDL3_ProcessEvent(event);
}

// Добавить в начало SDL_AppIterate
void Gui::IterateImGui()
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	// ImGui::ShowMetricsWindow();
	// ImGui::ShowDemoWindow();

	//---------------------------------
	if (showDebugWindow)
		DebugWindow();

	if (showFullscreenWindow)
		FullscreenWindow();

	if (showProtocolVMC)
		protocolVMC->WindowProtocol(showProtocolVMC);

	if (!showProtocolVMC && protocolVMC != nullptr)
		protocolVMC.reset();

	if (showNomogram)
		nomogram->NomogramWindow(showNomogram);

	if (!showNomogram && nomogram != nullptr)
		nomogram.reset();

	//---------------------------------

	// Для работы с HiDPI
	ImGuiIO& io = ImGui::GetIO();
	SDL_SetRenderScale(appData->renderer,
		io.DisplayFramebufferScale.x,
		io.DisplayFramebufferScale.y);
}

// Добавить перед SDL_RenderPresent
void Gui::RenderImGui()
{
	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), appData->renderer);
}

void Gui::DebugWindow()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Begin("DebugWindow", &showDebugWindow);
	{
		ImGui::Text("API: %s", SDL_GetRendererName(appData->renderer));
		ImGui::Text("Driver: %s", appData->driverName ? appData->driverName : "unknown");

		fpsUpdateTimer += io.DeltaTime;
		if (fpsUpdateTimer >= 0.5f)
		{
			currentFrametime = { 1000.f / io.Framerate };
			framerate = { io.Framerate };
			fpsUpdateTimer = { 0.f };
		}
		ImGui::Text("Application average %.2f ms/frame (%.0f FPS)", currentFrametime, framerate);

		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse pos: <INVALID>");
	}
	ImGui::End();
}

void Gui::FullscreenWindow()
{
	/* ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags = // ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus; */

		// Кнопка без окна///////////////////////////////////////////////
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
		// ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize |
		// ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoBackground;

	ImGui::Begin("Main Window", &showFullscreenWindow, window_flags);

	if (ImGui::Button("Создать заключение ВИК"))
	{
		showProtocolVMC = true;
		if (protocolVMC == nullptr)
			protocolVMC = std::make_unique<ProtocolVMC>();
	}

	if (ImGui::Button("Открыть номограмму РК"))
	{
		showNomogram = true;
		if (nomogram == nullptr)
			nomogram = std::make_unique<Nomogram>();
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
