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
	pdfManager.NewPage();

	double sumY = 0.0;
	double sumX = 0.0;
	Cell c10 = pdfManager.CreateCell(0, sumY, 5, {80, data.nameLabTitle});
	sumY += c10.h;
	Cell c20 = pdfManager.CreateCell(0, sumY, 10, {80, data.nameLab});
	sumY += c20.h;
	Cell c30 = pdfManager.CreateCell(0, sumY, 5, {80, data.numberAttestationTitle});
	sumY += c30.h;
	Cell c40 = pdfManager.CreateCell(0, sumY, 10, {80, data.numberAttestation});
	sumY += c40.h;

	sumX = c10.w;
	Cell c12 = pdfManager.CreateCell(sumX, 0, sumY, {80, data.protocolNumberTitle + data.protocolNumber + "\n" + "От " + data.protocolDate});

	sumX += c12.w;
	sumY = 0.0;
	Cell c13 = pdfManager.CreateCell(sumX, sumY, 10, {40, data.objectNameTitle});
	sumY += c13.h;
	Cell c23 = pdfManager.CreateCell(sumX, sumY, 5, {40, data.pipeCategoryTitle});
	sumY += c23.h;
	Cell c33 = pdfManager.CreateCell(sumX, sumY, 10, {40, data.contractorOrganizationTitle});
	sumY += c33.h;
	Cell c43 = pdfManager.CreateCell(sumX, sumY, 5, {40, data.customerOrganizationTitle});

	sumX += c43.w;
	sumY = 0.0;
	Cell c14 = pdfManager.CreateCell(sumX, sumY, c13.h, {0, data.objectName});
	sumY += c14.h;
	Cell c24 = pdfManager.CreateCell(sumX, sumY, c23.h, {0, data.pipeCategory.at(data.pipeCategoryIndex)});
	sumY += c24.h;
	Cell c34 = pdfManager.CreateCell(sumX, sumY, c33.h, {0, data.contractorOrganizationList.at(data.contractorOrganizationIndex)});
	sumY += c34.h;
	Cell c44 = pdfManager.CreateCell(sumX, sumY, c43.h, {0, data.customerOrganizationList.at(data.customerOrganizationIndex)});
	sumY += c44.h;

	pdfManager.cursorRowY = sumY;
	pdfManager.CreateRow(5, {{0, data.methodTitle}});
	pdfManager.CreateRow(5, {{0, data.technologicalControlMapTitle + " " + data.technologicalControlMap.at(data.technologicalControlMapIndex)}});
	pdfManager.CreateRow(5, {{0, data.normativeDocsTitle + " " + data.normativeDocs}});
	pdfManager.CreateRow(15, {{0, data.equipmentTitle + ": " + data.equipment}});

	pdfManager.CreateRow(5, {{.width = 0, .isRectVisible = false}});
	pdfManager.CreateRow(50, {{18, data.weldNumberTitle, 6.0},
							  {18, "hjdfjv"}});

	/* pdfManager.CreateRow(10, {
								 {100, data.masterNameTitle},
								 {40, data.masterNameList.at(data.masterNameIndex)},
								 {80, data.masterOrganization + ", " + data.masterCertNumber},
								 {20},
								 {45, data.protocolDate},
							 }); */

	pdfManager.SaveDocument();
}
