#include "reportBuilder.hpp"
#include "applicationData.hpp"
#include "protocolData.hpp"
#include "pdfManager.hpp"

ReportBuilder::ReportBuilder(ApplicationData &coreAppData, ProtocolData &protocolData)
    : appData{coreAppData}, data{protocolData}
{
}

void ReportBuilder::CreateReportRGC()
{
    PdfManager pdfManager(appData);
    pdfManager.StartTable();
    pdfManager.CreateRow(10, {{100, data.masterNameTitle},
                              {40, data.masterNameList.at(data.masterNameIndex)},
                              {80, data.masterOrganization + ", " + data.masterCertNumber},
                              {20},
                              {300, data.protocolDate}});
    pdfManager.EndTable();
}
