#pragma once

#include <vector>
#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class ReportData;

class ReportWindow
{
public:
	void ShowReportsWindow(std::vector<Report> &repList);

private:
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	bool reportCreateWindowIsOpen = false;
};