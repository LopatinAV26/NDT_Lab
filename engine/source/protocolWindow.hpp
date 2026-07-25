#pragma once

#include "protocol.hpp"
#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class ApplicationData;

class ProtocolWindow
{
public:
	explicit ProtocolWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	//void DefectTable();
	void ReportTable();
	//void ReportCreateWindow(ReportData &report, bool &isOpen);
	//void DefectCreateWindow(ReportData &report, bool &isOpen);

	std::vector<int> reportIndexesList; ///< Список индексов для печати в pdf

	ApplicationData &appData;
	Protocol protocol;
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	bool reportWindowIsOpen = false;
	//bool defectWindowIsOpen = false;
	int editingReportIndex = -1; ///< текущий индекс отчёта, который создаётся/редактируется

	int protocolTableRows = 0; ///< количество строк в таблице отчётов
	int protocolRow = 0; ///< текущий индекс отчёта в таблице отчётов
};