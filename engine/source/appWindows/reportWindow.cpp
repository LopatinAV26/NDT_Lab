#include "reportWindow.hpp"

#include <array>
#include <cfloat>
#include <cmath>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "ImGuiDatePicker.hpp"

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
					editWindow = true;
					editingIndex = row;
				}
			}

			if (ImGui::BeginPopupContextItem()) /// правый клик по строке - контекстное меню
			{
				if (ImGui::MenuItem("Редактировать"))
				{
					editWindow = true;
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

		static constexpr std::array<Method, 8> methods{
			Method::VT, Method::RT, Method::DRT, Method::UT,
			Method::PT, Method::LT, Method::MT, Method::DT,
		};

		for (Method value : methods)
			if (ImGui::RadioButton(GetMethodAbbreviation(value).c_str(), selectedMethod == value))
				selectedMethod = value;

		ImGui::Separator();
		if (ImGui::Button("Создать", ImVec2(120, 0)))
		{
			tableRows++;
			repList.resize(tableRows);
			repList.back().nameLab = lab.labInfo.laboratoryName;
			repList.back().numberAttestation = lab.labInfo.numberAttestation;
			repList.back().methodValue = selectedMethod;
			repList.back().methodHeader = repList.back().GetMethodReportTitle(selectedMethod);
			editWindow = true;
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

	if (editWindow && editingIndex >= 0 &&
		editingIndex < static_cast<int>(repList.size()))
		Edit(repList.at(editingIndex), editWindow, lab);
}

void ReportWindow::Edit(Report &report, bool &isOpen, Laboratory &lab)
{
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    const std::string windowTitle = "Заключение " + NDT::ToLowerUtf8(report.methodHeader);

    if (ImGui::Begin(windowTitle.c_str(), &isOpen, window_flags))
    {
        bool changed = false;

        if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("1");
            ImGui::TableSetupColumn("2");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::TextDisabled("Дата проведения контроля");
            {
                tm date = NDT::ParseIsoDateTm(report.controlDate);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::DatePicker("##Дата проведения контроля#", date))
                {
                    report.controlDate = NDT::FormatIsoDateTm(date);
                    report.reportDate = report.controlDate;
                    changed = true;
                }
            }

            ImGui::TextDisabled("Дата выдачи заключения");
            ImGui::SameLine();
            const bool dateMismatch = report.reportDate != report.controlDate;
            if (dateMismatch)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            NDT::HelpMarker("Дата на заключении по результатам НК должна соответствовать фактической дате проведения контроля");
            {
                tm date = NDT::ParseIsoDateTm(report.reportDate);
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::DatePicker("##Дата выдачи заключения#", date))
                {
                    report.reportDate = NDT::FormatIsoDateTm(date);
                    if (report.reportDate < report.controlDate)
                        report.reportDate = report.controlDate;
                    changed = true;
                }
                if (dateMismatch)
                    ImGui::PopStyleColor();
            }

            ImGui::TextDisabled("Номер сварного соединения");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Номер сварного соединения#", &report.weldNumber);

            ImGui::TextDisabled("Номер заключения");
            ImGui::SameLine();
            NDT::HelpMarker("");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Номер заключения#", &report.reportNumber);

            ImGui::TextDisabled("Наименование объекта");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Наименование объекта#", &report.objectName);

            ImGui::TextDisabled("Категория трубопровода");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Категория трубопровода#", report.pipeCategory.c_str()))
            {
                for (const auto &category : report.pipeCategoryList)
                {
                    const bool isSelected = (report.pipeCategory == category);
                    if (ImGui::Selectable(category.c_str(), isSelected))
                    {
                        report.pipeCategory = category;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Подрядная организация");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Подрядная организация#", &report.contractorOrganization);

            ImGui::TextDisabled("Организация заказчика");
            ImGui::SetNextItemWidth(-FLT_MIN);
            changed |= ImGui::InputText("##Организация заказчика#", &report.customerOrganization);

            if (ImGui::DragInt("##Диаметр#", &report.diameter, 1.f, 10, 1500, "Диаметр %d мм"))
            {
                if (report.diameter < 10)
                    report.diameter = 10;
                if (report.diameter > 1500)
                    report.diameter = 1500;
                report.perimeter = static_cast<int>(std::lround(report.diameter * 3.141592f));
                changed = true;
            }

            if (ImGui::DragInt("##Длина шва#", &report.perimeter, 1.f, 31, 4712, "Длина шва %d мм"))
            {
                if (report.perimeter < 31)
                    report.perimeter = 31;
                if (report.perimeter > 4712)
                    report.perimeter = 4712;
                report.diameter = static_cast<int>(std::lround(report.perimeter / 3.141592f));
                changed = true;
            }

            ImGui::TextDisabled("Контроль произвёл");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Контроль произвёл#", report.controllerName.c_str())) ////////////////////////////////////////
            {
                std::vector<Employee> empList = MethodFilter(lab.employeesList, report.methodValue, report.controlDate);
                for (const auto &employee : empList)
                {
                    if (employee.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.controllerName == employee.name);
                    if (ImGui::Selectable(employee.name.c_str(), isSelected))
                    {
                        report.controllerName = employee.name;
                        report.controllerOrganization = employee.organization;
                        report.controllerCertNumber = employee.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Заключение выдал");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Заключение выдал#", report.protocolCreateName.c_str())) ////////////////////////////////////////
            {
                std::vector<Employee> empList = MethodFilter(lab.employeesList, report.methodValue, report.reportDate);
                for (const auto &employee : empList)
                {
                    if (employee.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.protocolCreateName == employee.name);
                    if (ImGui::Selectable(employee.name.c_str(), isSelected))
                    {
                        report.protocolCreateName = employee.name;
                        report.protocolCreateOrganization = employee.organization;
                        report.protocolCreateCertNumber = employee.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Надзор");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Надзор#", report.inspectorName.c_str())) ////////////////////////////////////////
            {
                std::vector<Inspector> inspList = MethodFilter(lab.inspectorsList, report.methodValue, report.controlDate);
                const bool noneSelected = report.inspectorName.empty();
                if (ImGui::Selectable("—", noneSelected))
                {
                    report.inspectorName.clear();
                    report.inspectorOrganization.clear();
                    report.inspectorCertNumber.clear();
                    changed = true;
                }

                if (noneSelected)
                    ImGui::SetItemDefaultFocus();

                for (const auto &inspector : inspList)
                {
                    if (inspector.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.inspectorName == inspector.name);
                    if (ImGui::Selectable(inspector.name.c_str(), isSelected))
                    {
                        report.inspectorName = inspector.name;
                        report.inspectorOrganization = inspector.organization;
                        report.inspectorCertNumber = inspector.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::TextDisabled("Мастер");
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo("##Мастер#", report.masterName.c_str())) ////////////////////////////////////////
            {
                const bool noneSelected = report.masterName.empty();
                if (ImGui::Selectable("—", noneSelected))
                {
                    report.masterName.clear();
                    report.masterOrganization.clear();
                    report.masterCertNumber.clear();
                    changed = true;
                }

                if (noneSelected)
                    ImGui::SetItemDefaultFocus();

                for (const auto &master : lab.mastersList)
                {
                    if (master.deletedAt.has_value())
                        continue;

                    const bool isSelected = (report.masterName == master.name);
                    if (ImGui::Selectable(master.name.c_str(), isSelected))
                    {
                        report.masterName = master.name;
                        report.masterOrganization = master.organization;
                        report.masterCertNumber = master.certificateNumber;
                        changed = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::EndTable();
        }

        if (changed)
        {
            report.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        }

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.Show(report, defectWindowIsOpen);
}

std::vector<Employee> ReportWindow::MethodFilter(const std::vector<Employee> &lists, const Method method, const std::string &reportDate)
{
    const std::chrono::year_month_day reportYmd = NDT::ParseIsoDate(reportDate);

    std::vector<Employee> filteredVector;
    for (const Employee &employee : lists)
    {
        bool hasValidCertificate = false;
        switch (method)
        {
        case Method::VT:
            hasValidCertificate = employee.hasVT && NDT::ParseIsoDate(employee.certificateEndDateVT) > reportYmd;
            break;
        case Method::UT:
            hasValidCertificate = employee.hasUT && NDT::ParseIsoDate(employee.certificateEndDateUT) > reportYmd;
            break;
        case Method::RT:
            hasValidCertificate = employee.hasRT && NDT::ParseIsoDate(employee.certificateEndDateRT) > reportYmd;
            break;
        case Method::PT:
            hasValidCertificate = employee.hasPT && NDT::ParseIsoDate(employee.certificateEndDatePT) > reportYmd;
            break;
        case Method::MT:
            hasValidCertificate = employee.hasMT && NDT::ParseIsoDate(employee.certificateEndDateMT) > reportYmd;
            break;
        case Method::LT:
            hasValidCertificate = employee.hasLT && NDT::ParseIsoDate(employee.certificateEndDateLT) > reportYmd;
            break;
        }

        if (hasValidCertificate)
            filteredVector.push_back(employee);
    }

    return filteredVector;
}

std::vector<Inspector> ReportWindow::MethodFilter(const std::vector<Inspector> &lists, const Method method, const std::string &reportDate)
{
    const std::chrono::year_month_day reportYmd = NDT::ParseIsoDate(reportDate);

    std::vector<Inspector> filteredVector;
    for (const Inspector &inspector : lists)
    {
        bool hasValidCertificate = false;
        switch (method)
        {
        case Method::VT:
            hasValidCertificate = inspector.hasVT && NDT::ParseIsoDate(inspector.certificateEndDateVT) > reportYmd;
            break;
        case Method::UT:
            hasValidCertificate = inspector.hasUT && NDT::ParseIsoDate(inspector.certificateEndDateUT) > reportYmd;
            break;
        case Method::RT:
            hasValidCertificate = inspector.hasRT && NDT::ParseIsoDate(inspector.certificateEndDateRT) > reportYmd;
            break;
        case Method::PT:
            hasValidCertificate = inspector.hasPT && NDT::ParseIsoDate(inspector.certificateEndDatePT) > reportYmd;
            break;
        case Method::MT:
            hasValidCertificate = inspector.hasMT && NDT::ParseIsoDate(inspector.certificateEndDateMT) > reportYmd;
            break;
        case Method::LT:
            hasValidCertificate = inspector.hasLT && NDT::ParseIsoDate(inspector.certificateEndDateLT) > reportYmd;
            break;
        }

        if (hasValidCertificate)
            filteredVector.push_back(inspector);
    }

    return filteredVector;
}
