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
	std::vector<Employee> MethodFilter(const std::vector<Employee> &lists, const Method method, const std::string &reportDate);
	std::vector<Inspector> MethodFilter(const std::vector<Inspector> &lists, const Method method, const std::string &reportDate);
	

	ReportBuilder builder;
	DefectCreateWindow defectCreateWindow;

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoCollapse;

	bool defectWindowIsOpen = false;
	bool editWindow = false;
};
