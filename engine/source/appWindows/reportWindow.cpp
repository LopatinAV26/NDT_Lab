#include "reportWindow.hpp"

#include <cfloat>
#include <algorithm>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"

ReportWindow::ReportWindow(Laboratory &laboratory)
	: lnk{laboratory}
{
}

void ReportWindow::Show()
{
	ReportTable();

	if (ImGui::Button("Добавить отчёт"))
	{
		reportTableRows++;
		lnk.reportList.resize(reportTableRows);
		lnk.reportList.back().protocolDate = NDT::GetCurrentDateString();
		reportWindowIsOpen = true;
		editingReportIndex = reportTableRows - 1;
	}

	ImGui::SameLine();
	ImGui::BeginDisabled(reportIndexesList.empty());
	if (ImGui::Button("Удалить выбранные"))
	{
		std::sort(reportIndexesList.rbegin(), reportIndexesList.rend()); // по убыванию, чтобы стирать с конца
		for (int idx : reportIndexesList)
			lnk.reportList.erase(lnk.reportList.begin() + idx);
		reportIndexesList.clear();
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(reportIndexesList.empty());
	if (ImGui::Button("Сохранить выбранные в PDF"))
	{
		builder.BuildReportRGC(lnk.reportList, reportIndexesList);
	}
	ImGui::EndDisabled();

	if (reportWindowIsOpen && editingReportIndex >= 0 &&
		editingReportIndex < static_cast<int>(lnk.reportList.size()))
		reportCreateWindow.Show(lnk.reportList.at(editingReportIndex), reportWindowIsOpen);
}

void ReportWindow::ReportTable()
{
	if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 3
						  // ImGuiTableFlags_Borders |
						  // ImGuiTableFlags_NoHostExtendX
						  ))
	{
		ImGui::TableSetupColumn("Номер заключения");
		ImGui::TableSetupColumn("Дата заключения");
		ImGui::TableHeadersRow();

		reportTableRows = static_cast<int>(lnk.reportList.size());
		for (int row = 0; row < reportTableRows; ++row)
		{
			ReportData &repData = lnk.reportList[row];
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
			reportRow = row;
		}

		ImGui::EndTable();
	}
}
