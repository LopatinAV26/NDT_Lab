#pragma once

class ApplicationData;
class ProtocolData;

class ReportBuilder
{
public:
    explicit ReportBuilder(ProtocolData &protocolData);

    void CreateReportRGC(ApplicationData &coreAppData);

private:
    ProtocolData &data;
};