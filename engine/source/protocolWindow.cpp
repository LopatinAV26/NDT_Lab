#include "protocolWindow.hpp"
#include "applicationData.hpp"
#include "imgui.h"

ProtocolWindow::ProtocolWindow(ApplicationData &coreAppData)
	: appData{coreAppData},
	  pdf{appData}
{
}

void ProtocolWindow::Show(bool &isOpen)
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDecoration |
		// ImGuiWindowFlags_NoTitleBar |
		// ImGuiWindowFlags_NoMove |
		// ImGuiWindowFlags_NoResize |
		// ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Протокол контроля", &isOpen, window_flags))
	{
		ImGui::BeginChild("Header", ImVec2(0, 30));

		ImGui::Text("Протокол контроля");

		ImGui::SameLine(ImGui::GetWindowWidth() - 60);

		if (ImGui::Button("Выход"))
			isOpen = false;

		ImGui::EndChild();

		if (ImGui::Button("Создать PDF"))
		{
			pdf.CreateTableRGC(protocolData);
		}
		ImGui::TextLinkOpenURL("Открыть протокол", "test.pdf");
	}
	ImGui::End();
}
