#pragma once

#include "laboratory.hpp"
#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class ProtocolWindow
{
public:
	void Show(bool &isOpen);

private:
	void ReportTable();

	std::vector<int> reportIndexesList; ///< Список индексов для печати в pdf
	Protocol protocol;
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	int editingReportIndex = -1; ///< текущий индекс отчёта, который создаётся/редактируется
	int protocolTableRows = 0; ///< количество строк в таблице отчётов
	int protocolRow = 0; ///< текущий индекс отчёта в таблице отчётов
	bool reportWindowIsOpen = false;
};