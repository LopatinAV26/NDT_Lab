#pragma once

#include "protocol.hpp"
#include "reportBuilder.hpp"

class ApplicationData;

class ProtocolWindow
{
public:
	explicit ProtocolWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);
	void DefectTable();

private:
	ApplicationData &appData;
	Protocol protocol;
	ReportBuilder builder;
};