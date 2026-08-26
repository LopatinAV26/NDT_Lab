#include "reportWindow.hpp"

#include <cfloat>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"

void ReportWindow::Show(std::vector<Report> &repList, Laboratory &lab)
{
	static int tableRows = 0;			 ///< количество строк в таблице отчётов
	static int editingIndex = -1;		 ///< текущий индекс отчёта, который создаётся/редактируется
	static std::vector<int> indexesList; ///< Список индексов для печати в pdf
	static std::vector<bool> selected;

	if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2))
	{
		//ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
		// ImGui::TableSetupColumn("Номер заключения");
		// ImGui::TableSetupColumn("Дата заключения");
		// ImGui::TableHeadersRow();

		tableRows = static_cast<int>(repList.size());
		selected.resize(tableRows);

		static std::vector<int> visibleIndices; /// индексы repList, проходящие фильтр удаления
		visibleIndices.clear();
		for (int row = 0; row < tableRows; ++row)
		{
			if (repList.at(row).deletedAt.has_value()) /// если стоит временная метка, не отображаем в UI, т.к. запись помечена как удалённая
				continue;

			visibleIndices.push_back(row);
		}

		for (int row : visibleIndices)
		{
			ImGui::TableNextRow();
			ImGui::PushID(row);

			ImGui::TableNextColumn(); //--- чекбокс выбора для печати в PDF --------------------
			bool isSelected = selected.at(row);
			if (ImGui::Checkbox("##select", &isSelected))
			{
				selected.at(row) = isSelected;
				if (isSelected)
					indexesList.push_back(row);
				else
					std::erase(indexesList, row);
			}

			ImGui::SameLine();
			if (ImGui::Selectable(repList.at(row).reportNumber.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
			{
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					reportCreateWindowIsOpen = true;
					editingIndex = row;
				}
			}

			if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
			{
				if (ImGui::MenuItem("Редактировать"))
				{
					reportCreateWindowIsOpen = true;
					editingIndex = row;
				}
				if (ImGui::MenuItem("Удалить"))
				{
					repList.at(row).deletedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
					selected.at(row) = false;
					std::erase(indexesList, row);
				}
				ImGui::EndPopup();
			}

			ImGui::TableNextColumn(); //-----------------------------------------------------

			ImGui::TextUnformatted(NDT::FormatDateForDisplay(repList.at(row).reportDate).c_str());

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	static Method selectedMethod = Method::VT; ///< метод, выбранный в попапе создания заключения

	if (ImGui::Button("Создать заключение")) //////////////////////////////////////////
		ImGui::OpenPopup("Метод контроля");

	if (ImGui::BeginPopupModal("Метод контроля", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		//ImGui::TextUnformatted("Выберите метод контроля");
		//ImGui::Separator();

		static constexpr std::array<std::pair<const char *, Method>, 8> methods{{
			{"ВИК", Method::VT},
			{"РК", Method::RT},
			{"ЦРК", Method::DRT},
			{"УК", Method::UT},
			{"ПВК", Method::PT},
			{"ПВТ", Method::LT},
			{"МК", Method::MT},
			{"Расслоение", Method::DT},
		}};

		for (const auto &[label, value] : methods)
			if (ImGui::RadioButton(label, selectedMethod == value))
				selectedMethod = value;

		ImGui::Separator();
		if (ImGui::Button("Создать", ImVec2(120, 0)))
		{
			tableRows++;
			repList.resize(tableRows);
			repList.back().nameLab = lab.labInfo.laboratoryName;
			repList.back().numberAttestation = lab.labInfo.numberAttestation;
			repList.back().methodValue = selectedMethod;
			repList.back().method = repList.back().GetMethodTitle(selectedMethod);
			reportCreateWindowIsOpen = true;
			editingIndex = tableRows - 1;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if (ImGui::Button("Отмена", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

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