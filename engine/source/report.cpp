#include "report.hpp"

Report::Report()
{
    controlDate = NDT::GetCurrentIsoDate();
    reportDate = controlDate;
    pipeCategory = pipeCategoryList.at(pipeCategoryIndex);
}
