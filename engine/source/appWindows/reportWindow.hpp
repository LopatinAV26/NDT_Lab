#pragma once

#include <vector>
#include "reportBuilder.hpp"
#include "reportCreateWindow.hpp"

class ReportData;
class Laboratory;

class ReportWindow
{
public:
	void Show(std::vector<Report> &repList, Laboratory &lab);

private:
	ReportBuilder builder;
	ReportCreateWindow reportCreateWindow;

	bool reportCreateWindowIsOpen = false;
};