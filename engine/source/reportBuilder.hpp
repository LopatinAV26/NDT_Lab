#pragma once

#include "laboratory.hpp"

class ReportBuilder
{
public:
    explicit ReportBuilder();

    void BuildReportRGC(const std::vector<Report> &reportList, const std::vector<int> &reportIndexes);

private:
};