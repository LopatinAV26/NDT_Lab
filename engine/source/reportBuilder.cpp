#include "reportBuilder.hpp"

#include <format>
#include "pdfManager.hpp"

ReportBuilder::ReportBuilder()
{
}

void ReportBuilder::BuildReportRGC(const std::vector<ReportData> &reportList, const std::vector<int> &reportIndexes)
{
	for (const auto &index : reportIndexes)
	{
		const ReportData &reportData = reportList.at(index);
		PdfManager pdfManager;
		pdfManager.NewPage();

		double sumY = 0.0;
		double sumX = 0.0;
		Cell c10 = pdfManager.CreateCell(0, sumY, 4, {80, reportData.nameLabTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c10.h;
		Cell c20 = pdfManager.CreateCell(0, sumY, 16, {80, reportData.nameLab});
		sumY += c20.h;
		Cell c30 = pdfManager.CreateCell(0, sumY, 4, {80, reportData.numberAttestationTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c30.h;
		Cell c40 = pdfManager.CreateCell(0, sumY, 4, {80, reportData.numberAttestation});
		sumY += c40.h;

		sumX = c10.w;
		Cell c12 = pdfManager.CreateCell(sumX, 0, sumY, {90, reportData.protocolNumberTitle + reportData.protocolNumber + "\nОт " + reportData.protocolDate, 10, PoDoFo::PdfHorizontalAlignment::Center});

		sumX += c12.w;
		sumY = 0.0;
		Cell c13 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportData.objectNameTitle});
		sumY += c13.h;
		Cell c23 = pdfManager.CreateCell(sumX, sumY, 4, {40, reportData.pipeCategoryTitle});
		sumY += c23.h;
		Cell c33 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportData.contractorOrganizationTitle});
		sumY += c33.h;
		Cell c43 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportData.customerOrganizationTitle});

		sumX += c43.w;
		sumY = 0.0;
		Cell c14 = pdfManager.CreateCell(sumX, sumY, c13.h, {0, reportData.objectName});
		sumY += c14.h;
		Cell c24 = pdfManager.CreateCell(sumX, sumY, c23.h, {0, reportData.pipeCategory.at(reportData.pipeCategoryIndex)});
		sumY += c24.h;
		Cell c34 = pdfManager.CreateCell(sumX, sumY, c33.h, {0, reportData.contractorOrganizationList.at(reportData.contractorOrganizationIndex)});
		sumY += c34.h;
		Cell c44 = pdfManager.CreateCell(sumX, sumY, c43.h, {0, reportData.customerOrganizationList.at(reportData.customerOrganizationIndex)});
		sumY += c44.h;

		pdfManager.cursorRowY = sumY;
		pdfManager.CreateRow(5, {{0, reportData.methodTitle, 8, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Center, FontStyle::Bold}});
		pdfManager.CreateRow(4, {{0, reportData.technologicalControlMapTitle + " " + reportData.technologicalControlMap.at(reportData.technologicalControlMapIndex)}});
		pdfManager.CreateRow(4, {{0, reportData.normativeDocsTitle + " " + reportData.normativeDocs}});
		pdfManager.CreateRow(12, {{0, reportData.equipmentTitle + ": " + reportData.equipment, 6}});

		pdfManager.CreateRow(4, {{.width = 0, .isRectVisible = false}});
		std::vector<Cell> rowCells = pdfManager.CreateRow(35, {{18, reportData.weldNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportData.weldTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportData.diameterTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportData.weldersMarkTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportData.sectionTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportData.filmNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {15, reportData.sensitivityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {30, reportData.opticalDensityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {15, reportData.opticalDeltaTitle, 6, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Bottom},
															   {50, reportData.defectsTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportData.acceptableTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {0, reportData.notesTitle, 6, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateCell(rowCells.at(6).x, rowCells.at(6).y, 4, {rowCells.at(6).w + rowCells.at(7).w + rowCells.at(8).w, "Параметры снимка", 6, PoDoFo::PdfHorizontalAlignment::Center});

		pdfManager.CreateRow(4, {{rowCells.at(0).w, "1", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(1).w, "2", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(2).w, "3", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(3).w, "4", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(4).w, "5", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(5).w, "6", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(6).w, "7", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(7).w, "8", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(8).w, "9", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(9).w, "10", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(10).w, "11", 8, PoDoFo::PdfHorizontalAlignment::Center},
								 {rowCells.at(11).w, "12", 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(0.4, {{0}}); /// двойная граница

		///////////////////////////////////////
		// нарисовать таблицу переменной структуры
		/////////////////////////////////////

		pdfManager.CreateRow(4, {{.width = 0, .text = reportData.extentOfUnacceptableDefectsTitle + ": " + std::format("{:.1f}", reportData.extentOfUnacceptableDefects), .isRectVisible = false}});
		pdfManager.CreateRow(4, {{.width = 0, .text = reportData.controlResultTitle + ": " + reportData.controlResult.at(reportData.controlResultIndex), .isRectVisible = false}});

		double signHeight = 8;
		pdfManager.cursorRowY = pdfManager.yEnd - signHeight * 4.0;
		pdfManager.CreateRow(signHeight, {{90, reportData.controllerNameTitle, 8},
										  {40, reportData.controllerNameList.at(reportData.controllerNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportData.controllerOrganization + ", " + reportData.controllerCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportData.controlDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportData.protocolCreateNameTitle, 8},
										  {40, reportData.protocolCreateNameList.at(reportData.protocolCreateNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportData.protocolCreateOrganization + ", " + reportData.protocolCreateCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportData.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportData.inspectorNameTitle, 7},
										  {40, reportData.inspectorNameList.at(reportData.inspectorNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportData.inspectorOrganization + ", " + reportData.inspectorCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportData.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportData.masterNameTitle, 7},
										  {40, reportData.masterNameList.at(reportData.masterNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportData.masterOrganization + ", " + reportData.masterCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportData.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});

		pdfManager.NewPage();

		pdfManager.SaveDocument(std::format("{:s} {:s} от {:s}.pdf", NDT::GetCurrentDateString(), reportData.protocolNumber, reportData.protocolDate));
	}
}
