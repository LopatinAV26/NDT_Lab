#include "protocolWindow.hpp"
#include <cfloat>
#include <algorithm>
#include "applicationData.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"

ProtocolWindow::ProtocolWindow(ApplicationData &coreAppData)
	: appData{coreAppData}
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
		ReportCreateWindow(protocol.reportList.at(editingReportIndex), reportWindowIsOpen);

	if (defectWindowIsOpen && editingReportIndex >= 0 &&
		editingReportIndex < static_cast<int>(protocol.reportList.size()))
		DefectCreateWindow(protocol.reportList.at(editingReportIndex), defectWindowIsOpen);
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

		defectTableRows = static_cast<int>(protocol.reportList.at(editingReportIndex).defRGCList.size());
		for (int row = 0; row < defectTableRows; ++row)
		{
			DefRGC &def = protocol.reportList.at(editingReportIndex).defRGCList.at(row);
			ImGui::TableNextRow();
			ImGui::PushID(row);

			ImGui::TableSetColumnIndex(0); /////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputInt("##Координата#", &def.coord, 1, 100);
			if (def.coord < 0)
				def.coord = 0;

			ImGui::TableSetColumnIndex(1); //////////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##Обозначение#", def.name[def.nameIndex].c_str()))
			{
				for (int i = 0; i < std::ssize(def.name); ++i)
				{
					const bool isSelected = (def.nameIndex == i);
					if (ImGui::Selectable(def.name[i].c_str(), isSelected))
						def.nameIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			bool length = (def.name[def.nameIndex] == "Ac" || def.name[def.nameIndex] == "Ab") ? false : true;
			ImGui::BeginDisabled(length);
			ImGui::TableSetColumnIndex(2); /////////////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Протяжённость#", &def.length, 0.1f, 1.0f, "%.1f");
			if (def.length < 0.0f)
				def.length = 0.0f;
			ImGui::EndDisabled();

			ImGui::TableSetColumnIndex(3); //////////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Длина#", &def.width, 0.1f, 1.0f, "%.1f");
			if (def.width < 0.0f)
				def.width = 0.0f;

			ImGui::TableSetColumnIndex(4); //////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputFloat("##Ширина#", &def.height, 0.1f, 1.0f, "%.1f");
			if (def.height < 0.0f)
				def.height = 0.0f;

			ImGui::TableSetColumnIndex(5); //////////////////////////////////////////////////////
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::BeginCombo("##Окончание#", def.end[def.endIndex].c_str()))
			{
				for (int i = 0; i < std::ssize(def.end); ++i)
				{
					const bool isSelected = (def.endIndex == i);
					if (ImGui::Selectable(def.end[i].c_str(), isSelected))
						def.endIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::PopID();

			protocol.ConstructDefectRGCString(def);

			ImGui::TableSetColumnIndex(6); //////////////////////////////
			if (defectTableRows > 0)
			{
				// ImGui::Text("(%d) %s", data.defectList[row].coord, data.defectList[row].record.c_str());
				ImGui::TextUnformatted(std::format("({:d}) {}", def.coord, def.record).c_str()); // безопаснее
			}
		}

		ImGui::TableSetColumnIndex(7); //////////////////////////////
		if (defectTableRows > 0)
			if (ImGui::Button("Удалить дефект"))
			{
				defectTableRows--;
				protocol.reportList.at(editingReportIndex).defRGCList.resize(defectTableRows);
			}
		ImGui::EndTable();

		if (ImGui::Button("Добавить дефект"))
		{
			defectTableRows++;
			protocol.reportList.at(editingReportIndex).defRGCList.resize(defectTableRows);
		}
	}
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

void ProtocolWindow::ReportCreateWindow(ReportData &report, bool &isOpen)
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

	if (ImGui::Begin("Новое заключение", &isOpen, window_flags))
	{
		ImGui::InputText("Дата выдачи заключения", &report.protocolDate);
		ImGui::InputText("Номер заключения", &report.protocolNumber);

		if (ImGui::Button("Дефекты"))
		{
			defectWindowIsOpen = true;
		}
	}
	ImGui::End();
}

void ProtocolWindow::DefectCreateWindow(ReportData &report, bool &isOpen)
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

	if (ImGui::Begin("Конструктор дефектов", &isOpen, window_flags))
	{
		DefectTable();
	}
	ImGui::End();
}
