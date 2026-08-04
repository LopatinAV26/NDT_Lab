#include "reportWindow.hpp"

#include <cfloat>
#include <algorithm>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"

void ReportWindow::Show(std::vector<Report> &repList, Laboratory &lab)
{
	static int tableRows = 0;			   ///< количество строк в таблице отчётов
	static int editingIndex = -1;		   ///< текущий индекс отчёта, который создаётся/редактируется
	static std::vector<int> indexesList; ///< Список индексов для печати в pdf
	static std::vector<bool> selected;

	if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2))
	{
		//ImGui::TableSetupColumn("Номер заключения");
		//ImGui::TableSetupColumn("Дата заключения");
		//ImGui::TableHeadersRow();

		tableRows = static_cast<int>(repList.size());
		selected.resize(tableRows);
		for (int row = 0; row < tableRows; ++row)
		{
			ImGui::TableNextRow();
			ImGui::PushID(row);

			ImGui::TableNextColumn(); //-------------------------------------------------------
			if (ImGui::Selectable("###", selected.at(row), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					reportCreateWindowIsOpen = true;
					editingIndex = row;
				}
				else
				{
					selected.at(row) = !selected.at(row);
					if (selected.at(row))
						indexesList.push_back(row);
					else
						std::erase(indexesList, row);
				}
			}

			ImGui::SameLine();
			ImGui::TextUnformatted(std::format("{:s}", repList.at(row).reportNumber).c_str());

			ImGui::TableNextColumn(); //-----------------------------------------------------

			ImGui::TextUnformatted(NDT::FormatDateForDisplay(repList.at(row).reportDate).c_str());

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	if (ImGui::Button("+")) //////////////////////////////////////////
	{
		tableRows++;
		repList.resize(tableRows);
		reportCreateWindowIsOpen = true;
		editingIndex = tableRows - 1;
	}

	ImGui::SameLine();
	ImGui::BeginDisabled(indexesList.empty());
	if (ImGui::Button("Удалить выбранные")) ////////////////////////////////////////
	{
		std::sort(indexesList.rbegin(), indexesList.rend()); // по убыванию, чтобы стирать с конца
		for (int idx : indexesList)
			repList.erase(repList.begin() + idx);
		indexesList.clear();
		std::ranges::fill(selected, false); // индексы после удаления сдвинулись, старые флаги уже не соответствуют строкам
	}
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(indexesList.empty());
	if (ImGui::Button("Сохранить выбранные в PDF")) //////////////////////////////////////
	{
		builder.BuildReportRGC(repList, indexesList);
	}
	ImGui::EndDisabled();

	if (reportCreateWindowIsOpen && editingIndex >= 0 &&
		editingIndex < static_cast<int>(repList.size()))
		reportCreateWindow.Show(repList.at(editingIndex), reportCreateWindowIsOpen, lab);
}