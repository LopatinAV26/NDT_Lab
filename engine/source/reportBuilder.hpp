#pragma once

class ApplicationData;
class ProtocolData;

class ReportBuilder
{
public:
    explicit ReportBuilder(ApplicationData &coreAppData, ProtocolData &protocolData);

    void CreateReportRGC();

private:
    ApplicationData &appData;
    ProtocolData &data;
};