#include "protocolWindow.hpp"

#include <cfloat>
#include <algorithm>
#include "imgui.h"
#include "imgui_stdlib.h"

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

	if (ImGui::Begin("Отчёты НК", &isOpen, window_flags))
	{
		ReportTable();

		if (ImGui::Button("Добавить отчёт"))
		{
			protocolTableRows++;
			protocol.reportList.resize(protocolTableRows);
			protocol.reportList.back().protocolDate = NDT::GetCurrentDateString();
			reportWindowIsOpen = true;
			editingReportIndex = protocolTableRows - 1;
		}

		ImGui::SameLine();
		ImGui::BeginDisabled(reportIndexesList.empty());
		if (ImGui::Button("Удалить выбранные"))
		{
			std::sort(reportIndexesList.rbegin(), reportIndexesList.rend()); // по убыванию, чтобы стирать с конца
			for (int idx : reportIndexesList)
				protocol.reportList.erase(protocol.reportList.begin() + idx);
			reportIndexesList.clear();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::BeginDisabled(reportIndexesList.empty());
		if (ImGui::Button("Сохранить выбранные в PDF"))
		{
			builder.BuildReportRGC(protocol.reportList, reportIndexesList);
		}
		ImGui::EndDisabled();
	}
	ImGui::End();

	if (reportWindowIsOpen && editingReportIndex >= 0 &&
		editingReportIndex < static_cast<int>(protocol.reportList.size()))
		reportCreateWindow.Show(protocol.reportList.at(editingReportIndex), reportWindowIsOpen);
}

void ProtocolWindow::ReportTable()
{
	if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 3,
						  /*ImGuiTableFlags_Borders |*/ ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX))
	{
		ImGui::TableSetupColumn("Номер заключения");
		ImGui::TableSetupColumn("Дата заключения");
		ImGui::TableHeadersRow();

		protocolTableRows = static_cast<int>(protocol.reportList.size());
		for (int row = 0; row < protocolTableRows; ++row)
		{
			ReportData &repData = protocol.reportList[row];
			ImGui::TableNextRow();
			ImGui::PushID(row);

			ImGui::TableSetColumnIndex(0); /////////////////////////////////////////////////////////////////
			bool isSelected = std::find(reportIndexesList.begin(), reportIndexesList.end(), row) != reportIndexesList.end();
			if (ImGui::Checkbox("##select", &isSelected))
			{
				if (isSelected)
					reportIndexesList.push_back(row);
				else
					std::erase(reportIndexesList, row);
			}

			ImGui::SameLine();
			ImGui::TextUnformatted(std::format("{:s}", repData.protocolNumber).c_str());

			ImGui::TableSetColumnIndex(1); /////////////////////////////////////////////////////////////////
			ImGui::TextUnformatted(std::format("{:s}", repData.protocolDate).c_str());

			ImGui::SameLine();
			if (ImGui::Button("Редактировать"))
			{
				reportWindowIsOpen = true;
				editingReportIndex = row;
			}

			ImGui::PopID();
			protocolRow = row;
		}

		ImGui::EndTable();
	}
}
