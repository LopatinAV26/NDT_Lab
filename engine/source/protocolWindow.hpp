#pragma once

#include "protocolData.hpp"
#include "pdfManager.hpp"

class ApplicationData;

class ProtocolWindow
{
public:
	explicit ProtocolWindow(ApplicationData &coreAppData);
	void Show(bool &isOpen);

private:
	ApplicationData &appData;
	ProtocolData protocolData;
	PdfManager pdf;
};