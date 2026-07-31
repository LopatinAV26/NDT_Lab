#pragma once

#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class Laboratory;

class ReportWindow
{
public:
	ReportWindow(Laboratory &laboratory);
	void Show();

private:
	void ReportTable();

	std::vector<int> reportIndexesList; ///< Список индексов для печати в pdf
	Laboratory &lnk;
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	int editingReportIndex = -1; ///< текущий индекс отчёта, который создаётся/редактируется
	int reportTableRows = 0;	 ///< количество строк в таблице отчётов
	int reportRow = 0;			 ///< текущий индекс отчёта в таблице отчётов
	bool reportWindowIsOpen = false;
};