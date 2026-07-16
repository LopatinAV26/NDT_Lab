#pragma once

#include "protocolData.hpp"
#include "reportBuilder.hpp"

class ApplicationData;

class ProtocolWindow
{
public:
	explicit ProtocolWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	ApplicationData &appData;
	ProtocolData data;
	ReportBuilder builder;
};