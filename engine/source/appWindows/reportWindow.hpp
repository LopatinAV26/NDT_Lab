#pragma once

#include <vector>
#include <string>
#include "imgui.h"
#include "reportBuilder.hpp"
#include "defectCreateWindow.hpp"
#include "methodsNdt.hpp"

class Laboratory;
struct Employee;
struct Inspector;
struct ControlMap;

class ReportWindow
{
public:
	void Show(std::vector<Report> &repList, Laboratory &lab);

private:
	void Edit(Report &report, bool &isOpen, Laboratory &lab);

	/// @brief Вернуть список сотрудников, отфильтрованных по наличию действующего метода
	/// @param lists
	/// @param method метод, по которому нужно фильтровать сотрудников
	/// @param reportDate дата в формате ISO (YYYY-MM-DD), на которую нужно проверить действителен ли допуск
	/// @return отфильтрованный список сотрудников
	std::vector<Employee> MethodFilter(const std::vector<Employee> &lists, Method method, const std::string &reportDate);

	std::vector<Inspector> MethodFilter(const std::vector<Inspector> &lists, Method method, const std::string &reportDate);

	/// @brief Вернуть техкарты, подходящие по методу, диаметру, толщине стенки и категории трубопровода
	/// @param lists
	/// @param method метод контроля заключения
	/// @param diameter диаметр, мм
	/// @param thickness толщина стенки, мм
	/// @param category категория трубопровода
	/// @return отфильтрованный список техкарт
	std::vector<ControlMap> MethodFilter(const std::vector<ControlMap> &lists, Method method, int diameter, float thickness, Category category);

	/// @brief Вернуть действующие нормативные документы по методу; документы без выбранного метода
	/// @brief считаются общими и попадают в список для любого метода
	/// @param lists
	/// @param method метод контроля заключения
	/// @return отфильтрованный список документов
	std::vector<NormativeDocument> MethodFilter(const std::vector<NormativeDocument> &lists, Method method);

	/// @brief Вернуть массив оборудования, которое соответствует методу, поверено и исправно
	/// @param lists
	/// @param method
	/// @param reportDate
	/// @return
	std::vector<Equipment> MethodFilter(const std::vector<Equipment> &lists, Method method, const std::string &reportDate);

	/// @brief Собрать строку из отмеченного в комбобоксе оборудования: "Наименование №зав.номер" через запятую
	/// @param list отфильтрованное оборудование, из которого берутся отмеченные позиции
	/// @param selectedIds id выбранного оборудования (Report::equipmentIds)
	/// @return строка для заключения
	std::string GetEquipmentString(const std::vector<Equipment> &list, const std::vector<std::string> &selectedIds);

	/// @brief Собрать строку из шифров отмеченных нормативных документов через запятую
	/// @param list отфильтрованные документы, из которых берутся отмеченные позиции
	/// @param selectedIds id выбранных документов (Report::normativeDocsIds)
	/// @return строка для заключения
	std::string GetNormativeDocsString(const std::vector<NormativeDocument> &list, const std::vector<std::string> &selectedIds);

	ReportBuilder builder;
	DefectCreateWindow defectCreateWindow;

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	bool defectWindowIsOpen = false;
	bool editWindow = false;
};
