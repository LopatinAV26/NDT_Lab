#include "protocolWindow.hpp"
#include "applicationData.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

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
		// if (ImGui::BeginTable("", 6, ImGuiTableFlags_Borders))
		//{
		// ImGui::TableSetupColumn("small");
		// ImGui::TableSetupColumn("half");
		// ImGui::TableSetupColumn("right-align");
		// ImGui::TableHeadersRow();

		static int rows = 0;
		data.defectList.clear();
		for (int row = 0; row < rows; ++row)
		{
			DefInputRGC &defInput = data.defectInputRGCVector[row];
			Defect defect;
			// ImGui::TableNextRow();
			ImGui::PushID(row);

			// ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(150);
			if (ImGui::BeginCombo("##Обозначение#", defInput.name[defInput.nameIndex].c_str()))
			{
				for (int i = 0; i < std::ssize(defInput.name); ++i)
				{
					const bool isSelected = (defInput.nameIndex == i);
					if (ImGui::Selectable(defInput.name[i].c_str(), isSelected))
						defInput.nameIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::InputFloat("##Протяжённость#", &defInput.length, 0.1f, 1.0f, "%.1f");
			if (defInput.length < 0.0f)
				defInput.length = 0.0f;

			ImGui::SameLine();
			if (ImGui::BeginCombo("##Разделитель1#", defInput.spacer[defInput.spacerIndex1].c_str()))
			{
				for (int i = 0; i < std::ssize(defInput.spacer); ++i)
				{
					const bool isSelected = (defInput.spacerIndex1 == i);
					if (ImGui::Selectable(defInput.spacer[i].c_str(), isSelected))
						defInput.spacerIndex1 = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::InputFloat("##Длина#", &defInput.width, 0.1f, 1.0f, "%.1f");
			if (defInput.width < 0.0f)
				defInput.width = 0.0f;

			ImGui::SameLine();
			if (ImGui::BeginCombo("##Разделитель2#", defInput.spacer[defInput.spacerIndex2].c_str()))
			{
				for (int i = 0; i < std::ssize(defInput.spacer); ++i)
				{
					const bool isSelected = (defInput.spacerIndex2 == i);
					if (ImGui::Selectable(defInput.spacer[i].c_str(), isSelected))
						defInput.spacerIndex2 = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::InputFloat("##Ширина#", &defInput.height, 0.1f, 1.0f, "%.1f");
			if (defInput.height < 0.0f)
				defInput.height = 0.0f;

			ImGui::SameLine();
			if (ImGui::BeginCombo("##Окончание#", defInput.end[defInput.endIndex].c_str()))
			{
				for (int i = 0; i < std::ssize(defInput.end); ++i)
				{
					const bool isSelected = (defInput.endIndex == i);
					if (ImGui::Selectable(defInput.end[i].c_str(), isSelected))
						defInput.endIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::PopID();

			defect = data.CreateDefectRGC(defInput);
			data.defectList.push_back(defect);

			ImGui::SameLine();
			if (rows > 0)
				ImGui::Text(data.defectList[row].record.c_str());
		}

		// ImGui::EndTable();

		ImGui::SameLine();
		if (rows > 0)
			if (ImGui::Button("-"))
			{
				rows--;
				data.defectInputRGCVector.resize(rows);
			}

		if (ImGui::Button("+"))
		{
			rows++;
			data.defectInputRGCVector.resize(rows);
		}
		//}

		if (ImGui::Button("Создать PDF"))
		{
			builder.CreateReportRGC(appData);
		}
		ImGui::TextLinkOpenURL("Открыть протокол", "test.pdf");
	}
	ImGui::End();
}
