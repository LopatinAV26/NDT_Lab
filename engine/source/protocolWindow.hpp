#pragma once

#include "protocol.hpp"
#include "reportBuilder.hpp"

class ApplicationData;

class ProtocolWindow
{
public:
	explicit ProtocolWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	void DefectTable();
	void ReportTable();
	void ReportCreateWindow(ReportData &report, bool &isOpen);
	std::string GetCurrentDateString();

	std::vector<int> reportIndexesList; ///< Список индексов для печати в pdf

	ApplicationData &appData;
	Protocol protocol;
	ReportBuilder builder;

	bool reportWindowIsOpen = false;
	int editingReportIndex = -1; ///< текущий индекс отчёта, который создаётся/редактируется

	int protocolTableRows = 0; 
	int protocolRow = 0; ///< текущий индекс отчёта в таблице отчётов
};