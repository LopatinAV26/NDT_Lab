#include "protocolWindow.hpp"

#include "imgui.h"

void ProtocolWindow::Show(bool &isOpen)
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration;
	// ImGuiWindowFlags_NoTitleBar |
	// ImGuiWindowFlags_NoMove |
	// ImGuiWindowFlags_NoResize |
	// ImGuiWindowFlags_NoCollapse |
	// ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Протокол контроля", &isOpen, window_flags))
	{
#if defined(_WIN32)
		ImGui::BeginChild("Header", ImVec2(0, 30 * appData.mainScale));
#elif defined(__linux__)
		ImGui::BeginChild("Header", ImVec2(0, 30));
#else
		ImGui::BeginChild("Header", ImVec2(0, 30));
#endif

		ImGui::Text("Протокол контроля");

#if defined(_WIN32)
		ImGui::SameLine(ImGui::GetWindowWidth() - 60 * appData.mainScale);
#elif defined(__linux__)
		ImGui::SameLine(ImGui::GetWindowWidth() - 60);
#else
		ImGui::SameLine(ImGui::GetWindowWidth() - 60);
#endif

		if (ImGui::Button("Выход"))
			isOpen = false;

		ImGui::EndChild();

	if (ImGui::Button("Создать PDF")){
		pdf.CreateTableRGC(protocolData);
	}

	}
	ImGui::End();
}

ProtocolWindow::ProtocolWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
{
}
