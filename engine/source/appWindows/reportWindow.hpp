#pragma once

#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class Laboratory;

class ReportWindow
{
public:
	void Show(std::vector<ReportData> &repList);

private:
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	bool reportCreateWindowIsOpen = false;
};