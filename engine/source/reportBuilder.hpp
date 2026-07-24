#pragma once

#include "protocol.hpp"

class ReportBuilder
{
public:
    explicit ReportBuilder();

    void BuildReportRGC(const std::vector<ReportData> &reportList, const std::vector<int> &reportIndexes);

private:
};