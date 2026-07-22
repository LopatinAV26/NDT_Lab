#include "protocolWindow.hpp"
#include <cfloat>
#include "applicationData.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

ProtocolWindow::ProtocolWindow(ApplicationData &coreAppData)
	: appData{coreAppData},
	  builder{protocol}
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
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin("Протокол контроля", &isOpen, window_flags))
	{
		DefectTable();

		if (ImGui::Button("Создать PDF"))
		{
			builder.BuildReportRGC();
		}
		ImGui::TextLinkOpenURL("Открыть протокол", "test.pdf");
	}
	ImGui::End();
}

void ProtocolWindow::DefectTable()
{
	if (ImGui::BeginTable("Defect creator", 8, ImGuiTableFlags_Borders))
	{
		ImGui::TableSetupColumn("Координата");
		ImGui::TableSetupColumn("Обозначение");
		ImGui::TableSetupColumn("Протяжённость,\nдля Ac и Ab");
		ImGui::TableSetupColumn("Длина");
		ImGui::TableSetupColumn("Ширина");
		ImGui::TableSetupColumn("Превышение\nплотности");
		ImGui::TableSetupColumn("Запись дефекта");
		ImGui::TableHeadersRow();

		static int rows = 0;
		protocol.defectList.clear();
		for (int row = 0; row < rows; ++row)
		{
			DefRGCData &defInput = protocol.defectDataList[row];
			ImGui::TableNextRow();
			ImGui::PushID(row);

			ImGui::TableSetColumnIndex(0); /////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputInt("##Координата#", &defInput.coord, 1, 100);
			if (defInput.coord < 0)
				defInput.coord = 0;

			ImGui::TableSetColumnIndex(1); //////////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
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

			// bool length = false;
			bool length = (defInput.name[defInput.nameIndex] == "Ac" || defInput.name[defInput.nameIndex] == "Ab") ? false : true;
			ImGui::BeginDisabled(length);
			ImGui::TableSetColumnIndex(2); /////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Протяжённость#", &defInput.length, 0.1f, 1.0f, "%.1f");
			if (defInput.length < 0.0f)
				defInput.length = 0.0f;
			ImGui::EndDisabled();

			ImGui::TableSetColumnIndex(3); //////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Длина#", &defInput.width, 0.1f, 1.0f, "%.1f");
			if (defInput.width < 0.0f)
				defInput.width = 0.0f;

			ImGui::TableSetColumnIndex(4); //////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Ширина#", &defInput.height, 0.1f, 1.0f, "%.1f");
			if (defInput.height < 0.0f)
				defInput.height = 0.0f;

			ImGui::TableSetColumnIndex(5); //////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
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

			protocol.defectList.push_back(protocol.CreateDefectRGC(defInput));

			ImGui::TableSetColumnIndex(6); //////////////////////////////
			if (rows > 0)
			{
				// ImGui::Text("(%d) %s", data.defectList[row].coord, data.defectList[row].record.c_str());
				ImGui::TextUnformatted(std::format("({:d}) {}", protocol.defectList[row].coord, protocol.defectList[row].record).c_str());
			}
		}

		ImGui::TableSetColumnIndex(7); //////////////////////////////
		if (rows > 0)
			if (ImGui::Button("Удалить дефект"))
			{
				rows--;
				protocol.defectDataList.resize(rows);
			}
		ImGui::EndTable();

		if (ImGui::Button("Добавить дефект"))
		{
			rows++;
			protocol.defectDataList.resize(rows);
		}
	}
}
