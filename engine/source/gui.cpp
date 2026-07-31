#include "gui.hpp"
#include <SDL3/SDL.h>
#include "implot.h"
#include "embeddedFonts.hpp"
#include "applicationData.hpp"
#include "resourceManager.hpp"

Gui::Gui(ApplicationData &coreAppData, ResourceManager &resourceManager)
	: appData{coreAppData},
	  resManager{resourceManager},
	  settingsWindow{appData},
	  labWindow{appData, resManager}
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
	io.IniFilename = nullptr;

	if (ImFont *font = LoadEmbeddedShareTechMono(io, (appData.fontSizeMax * appData.mainScale)))
		io.FontDefault = font;
	io.FontGlobalScale = appData.fontSize / appData.fontSizeMax;

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

	// SetRadiationTheme();

	ImGuiStyle &style = ImGui::GetStyle();
#ifdef WIN32
	style.ScaleAllSizes(appData.mainScale);
#endif
	style.WindowRounding = appData.windowRounding;
	style.FrameRounding = appData.frameRounding;
	style.GrabRounding = appData.grabRounding;
	style.WindowBorderSize = 0.0f;

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

	if (labWindowIsOpen)
		labWindow.Show(labWindowIsOpen);
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

void Gui::SetRadiationTheme()
{
	ImGuiStyle &style = ImGui::GetStyle();
	ImVec4 *colors = style.Colors;

	// Базовые радиационно-жёлтые и ржаво-железные оттенки
	const ImVec4 rad_yellow = ImVec4(0.95f, 0.80f, 0.05f, 1.00f); // основной жёлтый
	const ImVec4 rad_yellow_dim = ImVec4(0.65f, 0.55f, 0.05f, 1.00f);
	const ImVec4 rust = ImVec4(0.55f, 0.28f, 0.10f, 1.00f); // ржавчина
	const ImVec4 rust_dark = ImVec4(0.30f, 0.16f, 0.08f, 1.00f);
	const ImVec4 iron_black = ImVec4(0.06f, 0.06f, 0.05f, 1.00f); // почти чёрный металл
	const ImVec4 iron_dark = ImVec4(0.11f, 0.10f, 0.08f, 1.00f);
	const ImVec4 iron_mid = ImVec4(0.18f, 0.16f, 0.12f, 1.00f);

	// Текст
	colors[ImGuiCol_Text] = rad_yellow;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.40f, 0.20f, 1.00f);

	// Фоны окон
	colors[ImGuiCol_WindowBg] = iron_black;
	colors[ImGuiCol_ChildBg] = iron_dark;
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.04f, 0.98f);

	// Границы — ржавые
	colors[ImGuiCol_Border] = rust;
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

	// Поля ввода
	colors[ImGuiCol_FrameBg] = iron_mid;
	colors[ImGuiCol_FrameBgHovered] = rust_dark;
	colors[ImGuiCol_FrameBgActive] = rust;

	// Заголовки окон
	colors[ImGuiCol_TitleBg] = iron_dark;
	colors[ImGuiCol_TitleBgActive] = rust_dark;
	colors[ImGuiCol_TitleBgCollapsed] = iron_black;

	colors[ImGuiCol_MenuBarBg] = iron_dark;

	// Скроллбар
	colors[ImGuiCol_ScrollbarBg] = iron_black;
	colors[ImGuiCol_ScrollbarGrab] = rust;
	colors[ImGuiCol_ScrollbarGrabHovered] = rad_yellow_dim;
	colors[ImGuiCol_ScrollbarGrabActive] = rad_yellow;

	// Чекбоксы / радио
	colors[ImGuiCol_CheckMark] = rad_yellow;

	// Слайдеры
	colors[ImGuiCol_SliderGrab] = rust;
	colors[ImGuiCol_SliderGrabActive] = rad_yellow;

	// Кнопки
	colors[ImGuiCol_Button] = rust_dark;
	colors[ImGuiCol_ButtonHovered] = rust;
	colors[ImGuiCol_ButtonActive] = rad_yellow_dim;

	// Заголовки (TreeNode, Selectable, CollapsingHeader)
	colors[ImGuiCol_Header] = rust_dark;
	colors[ImGuiCol_HeaderHovered] = rust;
	colors[ImGuiCol_HeaderActive] = rad_yellow_dim;

	// Разделители
	colors[ImGuiCol_Separator] = rust;
	colors[ImGuiCol_SeparatorHovered] = rad_yellow_dim;
	colors[ImGuiCol_SeparatorActive] = rad_yellow;

	// Ресайз-хваталки окон
	colors[ImGuiCol_ResizeGrip] = rust_dark;
	colors[ImGuiCol_ResizeGripHovered] = rust;
	colors[ImGuiCol_ResizeGripActive] = rad_yellow;

	// Вкладки
	colors[ImGuiCol_Tab] = iron_dark;
	colors[ImGuiCol_TabHovered] = rust;
	colors[ImGuiCol_TabActive] = rust_dark;
	colors[ImGuiCol_TabUnfocused] = iron_black;
	colors[ImGuiCol_TabUnfocusedActive] = iron_mid;

	// Графики
	colors[ImGuiCol_PlotLines] = rad_yellow;
	colors[ImGuiCol_PlotLinesHovered] = rad_yellow_dim;
	colors[ImGuiCol_PlotHistogram] = rust;
	colors[ImGuiCol_PlotHistogramHovered] = rad_yellow;

	// Таблицы
	colors[ImGuiCol_TableHeaderBg] = iron_mid;
	colors[ImGuiCol_TableBorderStrong] = rust;
	colors[ImGuiCol_TableBorderLight] = rust_dark;
	colors[ImGuiCol_TableRowBg] = iron_black;
	colors[ImGuiCol_TableRowBgAlt] = iron_dark;

	// Выделение текста
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.95f, 0.80f, 0.05f, 0.35f);

	// Drag&Drop цель
	colors[ImGuiCol_DragDropTarget] = rad_yellow;

	// Навигация / модалки
	colors[ImGuiCol_NavHighlight] = rad_yellow;
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.95f, 0.80f, 0.05f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.08f, 0.02f, 0.60f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.08f, 0.02f, 0.60f);

	// --- Форма элементов, чтобы было похоже на "тяжёлый металл" ---
	style.WindowRounding = 2.0f; // почти без скруглений — грубый металл
	style.FrameRounding = 1.0f;
	style.ScrollbarRounding = 1.0f;
	style.GrabRounding = 1.0f;
	style.TabRounding = 1.0f;

	style.WindowBorderSize = 1.5f; // толще границы — "клёпаный" вид
	style.FrameBorderSize = 1.0f;
	style.PopupBorderSize = 1.5f;

	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(8, 6);
	style.ScrollbarSize = 14.0f;
	style.GrabMinSize = 10.0f;
}

void Gui::ButtonsWindow()
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetCenter());

	// Кнопки без окна
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDecoration |
		// ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoBackground;

	if (ImGui::Begin("Main Window", &buttonsWindowOpen, window_flags))
	{
		if (ImGui::Button("Настройки"))
			settingsWindowOpen = true;

		if (ImGui::Button("Вход в ЛНК"))
			labWindowIsOpen = true;
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
