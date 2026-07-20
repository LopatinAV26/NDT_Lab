#include "protocolWindow.hpp"
#include "applicationData.hpp"
#include "imgui.h"

ProtocolWindow::ProtocolWindow(ApplicationData &coreAppData)
	: appData{coreAppData},
	  builder{data}
{
}

void ProtocolWindow::Show(bool &isOpen)
{
	ImGuiViewport *viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	ImGuiWindowFlags window_flags =
		//  ImGuiWindowFlags_NoDecoration |
		//  ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		//  ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Протокол контроля", &isOpen, window_flags))
	{
		if (ImGui::Button("Создать PDF"))
		{
			builder.CreateReportRGC(appData);
		}
		ImGui::TextLinkOpenURL("Открыть протокол", "test.pdf");
	}
	ImGui::End();
}
