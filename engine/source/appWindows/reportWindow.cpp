#include "reportWindow.hpp"

#include <array>
#include <algorithm>
#include <cmath>
#include <format>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "laboratory.hpp"
#include "utilities.hpp"
#include "ImGuiDatePicker.hpp"

void ReportWindow::Show(std::vector<Report> &repList, Laboratory &lab)
{
    static int tableRows = 0;            ///< количество строк в таблице отчётов
    static int editingIndex = -1;        ///< текущий индекс отчёта, который создаётся/редактируется
    static std::vector<int> indexesList; ///< Список индексов для печати в pdf
    static std::vector<bool> selected;

    if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2))
    {
        // ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        //  ImGui::TableSetupColumn("Номер заключения");
        //  ImGui::TableSetupColumn("Дата заключения");
        //  ImGui::TableHeadersRow();

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
        // ImGui::TextUnformatted("Выберите метод контроля");
        // ImGui::Separator();

        static constexpr std::array<Method, 8> methods{
            Method::VT,
            Method::RT,
            Method::DRT,
            Method::UT,
            Method::PT,
            Method::LT,
            Method::MT,
            Method::DT,
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

        /* if (ImGui::BeginTable("Отчёты по неразрушающему контролю", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("1");
            ImGui::TableSetupColumn("2");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn(); */

        /// ширина формы ограничена контентом - иначе полосы SeparatorText тянутся на всё окно
        ImGui::BeginChild("##Форма заключения", ImVec2(ImGui::GetFontSize() * 32.0f, 0.0f), ImGuiChildFlags_AutoResizeY);

        /// общая ширина контролов - иначе подписи справа выстраиваются рваной лесенкой
        ImGui::PushItemWidth(ImGui::GetFontSize() * 18.0f);

        ImGui::SeparatorText("Даты");

        {
            tm date = NDT::ParseIsoDateTm(report.controlDate);
            if (ImGui::DatePicker("Дата проведения контроля", date))
            {
                report.controlDate = NDT::FormatIsoDateTm(date);
                report.reportDate = report.controlDate;
                changed = true;
            }
        }

        const bool dateMismatch = report.reportDate != report.controlDate;
        {
            tm date = NDT::ParseIsoDateTm(report.reportDate);
            if (dateMismatch)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

            if (ImGui::DatePicker("Дата выдачи заключения", date))
            {
                report.reportDate = NDT::FormatIsoDateTm(date);
                if (report.reportDate < report.controlDate)
                    report.reportDate = report.controlDate;
                changed = true;
            }

            if (dateMismatch)
                ImGui::PopStyleColor();
        }
        ImGui::SameLine();
        NDT::HelpMarker("Дата на заключении по результатам НК должна соответствовать фактической дате проведения контроля");

        ImGui::SeparatorText("Объект контроля");

        changed |= ImGui::InputText("Номер сварного соединения", &report.weldNumber);

        changed |= ImGui::InputText("Номер заключения", &report.reportNumber);
        ImGui::SameLine();
        NDT::HelpMarker("");

        changed |= ImGui::InputText("Наименование объекта", &report.objectName);

        changed |= NDT::EnumCombo("Категория трубопровода", report.pipeCategory, GetCategoryStr);

        ImGui::SeparatorText("Организации");

        changed |= ImGui::InputText("Подрядная организация", &report.contractorOrganization);

        changed |= ImGui::InputText("Организация заказчика", &report.customerOrganization);

        ImGui::SeparatorText("Параметры сварного соединения");

        changed |= NDT::EnumCombo("Тип соединения", report.weldType, GetWeldJointTypeStr);

        /// стык варят комбинацией способов - корень одним, заполнение другим, поэтому чекбоксы,
        /// а в превью сразу собранная строка: обозначения короткие, счётчик здесь ничего не даёт
        const std::string weldingMethodsPreview = GetWeldingMethodsStr(report.weldingMethods);
        changed |= NDT::EnumCheckboxCombo("Способ сварки", weldingMethodsPreview.c_str(),
                                          report.weldingMethods, GetWeldingMethodStr, GetWeldingMethodName);

        if (ImGui::DragInt("Диаметр", &report.diameter, 1.f, 10, 1500, "%d мм"))
        {
            if (report.diameter < 10)
                report.diameter = 10;
            if (report.diameter > 1500)
                report.diameter = 1500;
            report.perimeter = static_cast<int>(std::lround(report.diameter * 3.141592f));
            report.technologicalControlMap = "";
            changed = true;
        }

        if (ImGui::DragInt("Длина шва", &report.perimeter, 1.f, 31, 4712, "%d мм"))
        {
            if (report.perimeter < 31)
                report.perimeter = 31;
            if (report.perimeter > 4712)
                report.perimeter = 4712;
            report.diameter = static_cast<int>(std::lround(report.perimeter / 3.141592f));
            report.technologicalControlMap = "";
            changed = true;
        }

        /// толщины свариваемых элементов могут различаться - по наименьшей из них подбирается техкарта
        if (ImGui::DragFloat("Толщина 1", &report.thicknes1, 0.1f, 1.0f, 50.0f, "%.1f мм"))
        {
            report.thicknes1 = std::clamp(report.thicknes1, 1.0f, 50.0f);
            report.technologicalControlMap = "";
            changed = true;
        }

        if (ImGui::DragFloat("Толщина 2", &report.thicknes2, 0.1f, 1.0f, 50.0f, "%.1f мм"))
        {
            report.thicknes2 = std::clamp(report.thicknes2, 1.0f, 50.0f);
            report.technologicalControlMap = "";
            changed = true;
        }

        ImGui::InputText("Номер детали 1", &report.sectionNumber1);
        ImGui::InputText("Номер детали 2", &report.sectionNumber2);

        

        /// стык может варить бригада - отмечаем всех, кто на нём работал;
        /// в превью шифры через пробел, в бланк каждый уходит с новой строки
        std::vector<Welder> welderList = MethodFilter(lab.weldersList, report.controlDate);
        const std::string weldersPreview = GetWeldersMarkString(welderList, report.weldersIds, " ");
        if (ImGui::BeginCombo("Шифр клейма", weldersPreview.c_str()))
        {
            for (const auto &welder : welderList)
            {
                if (welder.deletedAt.has_value())
                    continue;

                ImGui::PushID(welder.id.c_str());

                bool isSelected = std::ranges::find(report.weldersIds, welder.id) != report.weldersIds.end();
                if (ImGui::Checkbox(welder.personalCode.c_str(), &isSelected))
                {
                    if (isSelected)
                        report.weldersIds.push_back(welder.id);
                    else
                        std::erase(report.weldersIds, welder.id);

                    report.weldersMark = GetWeldersMarkString(welderList, report.weldersIds, "\n");
                    changed = true;
                }

                /// в списке только шифр клейма - фамилию показываем подсказкой
                if (!welder.name.empty())
                    ImGui::SetItemTooltip("%s", welder.name.c_str());

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        /// количество доступных полей координат задаёт тип секции: у бесшовной и фланца
        /// продольных швов нет, у одношовной и перехода один, у двухшовной два.
        /// При смене типа лишние координаты обнуляем - иначе в бланк уйдёт значение от прежнего типа
        const int seamCount1 = GetSeamCount(report.sectionType1);
        if (NDT::EnumCombo("Тип секции 1", report.sectionType1, GetSectionTypeStr))
        {
            if (GetSeamCount(report.sectionType1) < 2)
                report.coordSec1Weld2 = 0;
            if (GetSeamCount(report.sectionType1) < 1)
                report.coordSec1Weld1 = 0;
            changed = true;
        }

        ImGui::BeginDisabled(seamCount1 < 1);
        changed |= ImGui::DragInt("Секция 1, шов 1", &report.coordSec1Weld1, 1.f, 0, report.perimeter, "%d мм");
        ImGui::EndDisabled();

        ImGui::BeginDisabled(seamCount1 < 2);
        changed |= ImGui::DragInt("Секция 1, шов 2", &report.coordSec1Weld2, 1.f, 0, report.perimeter, "%d мм");
        ImGui::EndDisabled();

        const int seamCount2 = GetSeamCount(report.sectionType2);
        if (NDT::EnumCombo("Тип секции 2", report.sectionType2, GetSectionTypeStr))
        {
            if (GetSeamCount(report.sectionType2) < 2)
                report.coordSec2Weld2 = 0;
            if (GetSeamCount(report.sectionType2) < 1)
                report.coordSec2Weld1 = 0;
            changed = true;
        }

        ImGui::BeginDisabled(seamCount2 < 1);
        changed |= ImGui::DragInt("Секция 2, шов 1", &report.coordSec2Weld1, 1.f, 0, report.perimeter, "%d мм");
        ImGui::EndDisabled();

        ImGui::BeginDisabled(seamCount2 < 2);
        changed |= ImGui::DragInt("Секция 2, шов 2", &report.coordSec2Weld2, 1.f, 0, report.perimeter, "%d мм");
        ImGui::EndDisabled();

        /// величина вычисляемая, руками не правится - поэтому LabelText, а не поле ввода
        const std::optional<int> minSeamDistance = report.GetMinSeamDistance();
        if (minSeamDistance.has_value())
            ImGui::LabelText("Расстояние между швами", "%d мм", minSeamDistance.value());
        else
            ImGui::LabelText("Расстояние между швами", "-");

        ImGui::SeparatorText("Нормативные документы");

        /// в превью держим счётчик, а не собранную строку - выбранные документы показаны ниже
        const std::string docsPreview = std::format("Выбрано: {}", report.normativeDocsIds.size());
        if (ImGui::BeginCombo("Оценка качества по", docsPreview.c_str()))
        {
            std::vector<NormativeDocument> docList = MethodFilter(lab.normativeDocumentsList, report.methodValue);
            for (const auto &document : docList)
            {
                if (document.deletedAt.has_value())
                    continue;

                ImGui::PushID(document.id.c_str());

                bool isSelected = std::ranges::find(report.normativeDocsIds, document.id) != report.normativeDocsIds.end();
                if (ImGui::Checkbox(document.code.c_str(), &isSelected))
                {
                    if (isSelected)
                        report.normativeDocsIds.push_back(document.id);
                    else
                        std::erase(report.normativeDocsIds, document.id);

                    report.normativeDocs = GetNormativeDocsString(docList, report.normativeDocsIds);
                    changed = true;
                }

                if (!document.name.empty()) /// в списке только шифр - полное наименование показываем подсказкой
                    ImGui::SetItemTooltip("%s", document.name.c_str());

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        /// выбранные документы - в рамке шириной с остальные контролы, со скроллом по вертикали
        if (ImGui::BeginChild("##Выбранные документы", ImVec2(ImGui::CalcItemWidth(), ImGui::GetTextLineHeightWithSpacing() * 3.0f), ImGuiChildFlags_Borders))
            ImGui::TextWrapped("%s", report.normativeDocs.c_str());
        ImGui::EndChild();

        ImGui::SeparatorText("Технологическая карта");

        if (ImGui::BeginCombo("Техкарта", report.technologicalControlMap.c_str()))
        {
            /// у свариваемых элементов толщины могут различаться - подбираем техкарту по наименьшей
            const float nominalWallThickness = std::min(report.thicknes1, report.thicknes2);
            std::vector<ControlMap> mapList = MethodFilter(lab.controlMapsList, report.methodValue, report.diameter, nominalWallThickness, report.pipeCategory);
            for (const auto &controlMap : mapList)
            {
                if (controlMap.deletedAt.has_value())
                    continue;

                ImGui::PushID(controlMap.id.c_str());

                const bool isSelected = (report.technologicalControlMap == controlMap.code);
                if (ImGui::Selectable(controlMap.code.c_str(), isSelected))
                {
                    report.technologicalControlMap = controlMap.code;
                    changed = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        ImGui::SeparatorText("Оборудование и материалы");

        /// в превью держим счётчик, а не собранную строку - выбранное оборудование показано ниже
        const std::string equipmentPreview = std::format("Выбрано: {}", report.equipmentIds.size());
        if (ImGui::BeginCombo("Оборудование", equipmentPreview.c_str())) ////////////////////////////////////////
        {
            std::vector<Equipment> equipList = MethodFilter(lab.equpmentsList, report.methodValue, report.controlDate);
            for (const auto &equipment : equipList)
            {
                if (equipment.deletedAt.has_value())
                    continue;

                ImGui::PushID(equipment.id.c_str()); /// иначе у приборов с одинаковым наименованием чекбоксы получат один ID

                bool isSelected = std::ranges::find(report.equipmentIds, equipment.id) != report.equipmentIds.end();
                if (ImGui::Checkbox(equipment.name.c_str(), &isSelected))
                {
                    if (isSelected)
                        report.equipmentIds.push_back(equipment.id);
                    else
                        std::erase(report.equipmentIds, equipment.id);

                    report.equipment = GetEquipmentString(equipList, report.equipmentIds);
                    changed = true;
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        /// список выбранного оборудования - в рамке шириной с остальные контролы, со скроллом по вертикали
        if (ImGui::BeginChild("##Выбранное оборудование", ImVec2(ImGui::CalcItemWidth(), ImGui::GetTextLineHeightWithSpacing() * 4.0f), ImGuiChildFlags_Borders))
            ImGui::TextWrapped("%s", report.equipment.c_str());
        ImGui::EndChild();

        ImGui::SeparatorText("Исполнители");

        if (ImGui::BeginCombo("Контроль произвёл", report.controllerName.c_str())) ////////////////////////////////////////
        {
            std::vector<Employee> empList = MethodFilter(lab.employeesList, report.methodValue, report.controlDate);
            for (const auto &employee : empList)
            {
                if (employee.deletedAt.has_value())
                    continue;

                ImGui::PushID(employee.id.c_str()); /// иначе у полных тёзок пункты списка получат один ID

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

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Заключение выдал", report.protocolCreateName.c_str())) ////////////////////////////////////////
        {
            std::vector<Employee> empList = MethodFilter(lab.employeesList, report.methodValue, report.reportDate);
            for (const auto &employee : empList)
            {
                if (employee.deletedAt.has_value())
                    continue;

                ImGui::PushID(employee.id.c_str());

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

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Надзор", report.inspectorName.c_str())) ////////////////////////////////////////
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

                ImGui::PushID(inspector.id.c_str());

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

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("Мастер", report.masterName.c_str())) ////////////////////////////////////////
        {
            std::vector<Master> masterList = MethodFilter(lab.mastersList, report.controlDate);
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

            for (const auto &master : masterList)
            {
                if (master.deletedAt.has_value())
                    continue;

                ImGui::PushID(master.id.c_str());

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

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        ImGui::EndChild();

        // ImGui::EndTable();
        //}

        if (changed)
        {
            report.updatedAt = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        }

        ImGui::Separator();

        if (ImGui::Button("Дефекты"))
            defectWindowIsOpen = true;
    }
    ImGui::End();

    if (defectWindowIsOpen)
        defectCreateWindow.Show(report, defectWindowIsOpen);
}

std::vector<Employee> ReportWindow::MethodFilter(const std::vector<Employee> &lists, Method method, const std::string &reportDate)
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

std::vector<Inspector> ReportWindow::MethodFilter(const std::vector<Inspector> &lists, Method method, const std::string &controlDate)
{
    const std::chrono::year_month_day controlYmd = NDT::ParseIsoDate(controlDate);

    std::vector<Inspector> filteredVector;
    for (const Inspector &inspector : lists)
    {
        bool hasValidCertificate = false;
        switch (method)
        {
        case Method::VT:
            hasValidCertificate = inspector.hasVT && NDT::ParseIsoDate(inspector.certificateEndDateVT) > controlYmd;
            break;
        case Method::UT:
            hasValidCertificate = inspector.hasUT && NDT::ParseIsoDate(inspector.certificateEndDateUT) > controlYmd;
            break;
        case Method::RT:
            hasValidCertificate = inspector.hasRT && NDT::ParseIsoDate(inspector.certificateEndDateRT) > controlYmd;
            break;
        case Method::PT:
            hasValidCertificate = inspector.hasPT && NDT::ParseIsoDate(inspector.certificateEndDatePT) > controlYmd;
            break;
        case Method::MT:
            hasValidCertificate = inspector.hasMT && NDT::ParseIsoDate(inspector.certificateEndDateMT) > controlYmd;
            break;
        case Method::LT:
            hasValidCertificate = inspector.hasLT && NDT::ParseIsoDate(inspector.certificateEndDateLT) > controlYmd;
            break;
        }

        if (hasValidCertificate)
            filteredVector.push_back(inspector);
    }

    return filteredVector;
}

std::vector<Equipment> ReportWindow::MethodFilter(const std::vector<Equipment> &lists, Method method, const std::string &controlDate)
{
    const std::chrono::year_month_day controlYmd = NDT::ParseIsoDate(controlDate);

    std::vector<Equipment> filteredVector;
    for (const Equipment &equipment : lists)
    {
        bool forMethod = false;
        switch (method)
        {
        case Method::VT:
            forMethod = equipment.forVT;
            break;
        case Method::UT:
            forMethod = equipment.forUT;
            break;
        case Method::RT:
            forMethod = equipment.forRT;
            break;
        case Method::DRT:
            forMethod = equipment.forDRT;
            break;
        case Method::PT:
            forMethod = equipment.forPT;
            break;
        case Method::MT:
            forMethod = equipment.forMT;
            break;
        case Method::LT:
            forMethod = equipment.forLT;
            break;
        case Method::ECT:
            forMethod = equipment.forECT;
            break;
        }

        const bool calibrationValid = !equipment.isCalibrated || NDT::ParseIsoDate(equipment.certificateEndDate) > controlYmd;

        if (forMethod && calibrationValid && equipment.isOperational)
            filteredVector.push_back(equipment);
    }

    return filteredVector;
}

std::vector<ControlMap> ReportWindow::MethodFilter(const std::vector<ControlMap> &lists, Method method, int diameter, float nominalWallThickness, Category category)
{
    std::vector<ControlMap> filteredVector;
    for (const ControlMap &controlMap : lists)
    {
        bool forMethod = false;
        switch (method)
        {
        case Method::VT:
            forMethod = controlMap.forVT;
            break;
        case Method::UT:
            forMethod = controlMap.forUT;
            break;
        case Method::RT:
            forMethod = controlMap.forRT;
            break;
        case Method::DRT:
            forMethod = controlMap.forDRT;
            break;
        case Method::PT:
            forMethod = controlMap.forPT;
            break;
        case Method::MT:
            forMethod = controlMap.forMT;
            break;
        case Method::LT:
            forMethod = controlMap.forLT;
            break;
        case Method::DT:
            forMethod = controlMap.forDT;
            break;
        }

        bool forCategory = false;
        switch (category)
        {
        case Category::H:
            forCategory = controlMap.categoryB;
            break;
        case Category::I:
            forCategory = controlMap.categoryI;
            break;
        case Category::II:
            forCategory = controlMap.categoryII;
            break;
        case Category::III:
            forCategory = controlMap.categoryIII;
            break;
        case Category::IV:
            forCategory = controlMap.categoryIV;
            break;
        }

        /// диаметр и толщина заданы в техкарте одним значением, поэтому сравниваем на совпадение;
        /// толщину - с допуском, т.к. в форме она вводится с одним знаком после запятой
        const bool sizeMatches = controlMap.diameter == diameter && std::fabs(controlMap.nominalWallThickness - nominalWallThickness) < 0.05f;

        if (forMethod && forCategory && sizeMatches)
            filteredVector.push_back(controlMap);
    }

    return filteredVector;
}

std::vector<NormativeDocument> ReportWindow::MethodFilter(const std::vector<NormativeDocument> &lists, Method method)
{
    std::vector<NormativeDocument> filteredVector;
    for (const NormativeDocument &document : lists)
    {
        if (document.status != "действующий")
            continue;

        bool forMethod = false;
        switch (method)
        {
        case Method::VT:
            forMethod = document.forVT;
            break;
        case Method::UT:
            forMethod = document.forUT;
            break;
        case Method::RT:
            forMethod = document.forRT;
            break;
        case Method::DRT:
            forMethod = document.forDRT;
            break;
        case Method::PT:
            forMethod = document.forPT;
            break;
        case Method::MT:
            forMethod = document.forMT;
            break;
        case Method::LT:
            forMethod = document.forLT;
            break;
        case Method::ECT:
            forMethod = document.forECT;
            break;
        }

        /// документ без выбранных методов считаем общим - он подходит к заключению по любому методу
        const bool commonDocument = !document.forVT && !document.forUT && !document.forRT && !document.forDRT &&
                                    !document.forPT && !document.forMT && !document.forLT && !document.forECT;

        if (forMethod || commonDocument)
            filteredVector.push_back(document);
    }

    return filteredVector;
}

std::vector<Master> ReportWindow::MethodFilter(const std::vector<Master> &lists, const std::string &controlDate)
{
    const std::chrono::year_month_day controlYmd = NDT::ParseIsoDate(controlDate);

    std::vector<Master> filteredVector;
    for (const Master &master : lists)
    {
        /// незаполненный срок разбирается в 2000-01-01 и сравнение не проходит - отдельная проверка не нужна
        if (!master.certificateNumber.empty() && NDT::ParseIsoDate(master.certificateEndDate) > controlYmd)
            filteredVector.push_back(master);
    }

    return filteredVector;
}

std::vector<Welder> ReportWindow::MethodFilter(const std::vector<Welder> &lists, const std::string &controlDate)
{
    const std::chrono::year_month_day controlYmd = NDT::ParseIsoDate(controlDate);

    std::vector<Welder> filteredVector;
    for (const Welder &welder : lists)
    {
        if (!welder.certificateNumber.empty() && NDT::ParseIsoDate(welder.certificateEndDate) > controlYmd)
            filteredVector.push_back(welder);
    }

    return filteredVector;
}

std::string ReportWindow::GetEquipmentString(const std::vector<Equipment> &list, const std::vector<std::string> &selectedIds)
{
    std::string result;
    for (const Equipment &equipment : list)
    {
        if (std::ranges::find(selectedIds, equipment.id) == selectedIds.end())
            continue;

        if (!result.empty())
            result += ", ";
        result += equipment.name + " зав.№" + equipment.serialNumber;
    }

    return result;
}

std::string ReportWindow::GetWeldersMarkString(const std::vector<Welder> &list, const std::vector<std::string> &selectedIds, const std::string &separator)
{
    std::string result;
    for (const Welder &welder : list)
    {
        if (std::ranges::find(selectedIds, welder.id) == selectedIds.end())
            continue;

        if (!result.empty())
            result += separator;
        result += welder.personalCode;
    }

    return result;
}

std::string ReportWindow::GetNormativeDocsString(const std::vector<NormativeDocument> &list, const std::vector<std::string> &selectedIds)
{
    std::string result;
    for (const NormativeDocument &document : list)
    {
        if (std::ranges::find(selectedIds, document.id) == selectedIds.end())
            continue;

        if (!result.empty())
            result += ", ";
        result += document.code;
    }

    return result;
}
