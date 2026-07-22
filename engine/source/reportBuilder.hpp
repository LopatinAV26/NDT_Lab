#pragma once

class Protocol;

class ReportBuilder
{
public:
    explicit ReportBuilder(Protocol &protocolData);

    void BuildReportRGC();

private:
    Protocol &protocol;
};