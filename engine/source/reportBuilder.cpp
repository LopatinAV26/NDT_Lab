#include "reportBuilder.hpp"

#include <format>
#include <chrono>
#include "pdfManager.hpp"

ReportBuilder::ReportBuilder()
{
}

void ReportBuilder::BuildReportRGC(const std::vector<ReportData> &reportList, const std::vector<int> reportIndexes)
{
	for (const auto &index : reportIndexes)
	{
		PdfManager pdfManager;
		pdfManager.NewPage();

		double sumY = 0.0;
		double sumX = 0.0;
		Cell c10 = pdfManager.CreateCell(0, sumY, 4, {80, reportList.at(index).nameLabTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c10.h;
		Cell c20 = pdfManager.CreateCell(0, sumY, 16, {80, reportList.at(index).nameLab});
		sumY += c20.h;
		Cell c30 = pdfManager.CreateCell(0, sumY, 4, {80, reportList.at(index).numberAttestationTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c30.h;
		Cell c40 = pdfManager.CreateCell(0, sumY, 4, {80, reportList.at(index).numberAttestation});
		sumY += c40.h;

		sumX = c10.w;
		Cell c12 = pdfManager.CreateCell(sumX, 0, sumY, {90, reportList.at(index).protocolNumberTitle + reportList.at(index).protocolNumber + "\nОт " + reportList.at(index).protocolDate, 10, PoDoFo::PdfHorizontalAlignment::Center});

		sumX += c12.w;
		sumY = 0.0;
		Cell c13 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportList.at(index).objectNameTitle});
		sumY += c13.h;
		Cell c23 = pdfManager.CreateCell(sumX, sumY, 4, {40, reportList.at(index).pipeCategoryTitle});
		sumY += c23.h;
		Cell c33 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportList.at(index).contractorOrganizationTitle});
		sumY += c33.h;
		Cell c43 = pdfManager.CreateCell(sumX, sumY, 8, {40, reportList.at(index).customerOrganizationTitle});

		sumX += c43.w;
		sumY = 0.0;
		Cell c14 = pdfManager.CreateCell(sumX, sumY, c13.h, {0, reportList.at(index).objectName});
		sumY += c14.h;
		Cell c24 = pdfManager.CreateCell(sumX, sumY, c23.h, {0, reportList.at(index).pipeCategory.at(reportList.at(index).pipeCategoryIndex)});
		sumY += c24.h;
		Cell c34 = pdfManager.CreateCell(sumX, sumY, c33.h, {0, reportList.at(index).contractorOrganizationList.at(reportList.at(index).contractorOrganizationIndex)});
		sumY += c34.h;
		Cell c44 = pdfManager.CreateCell(sumX, sumY, c43.h, {0, reportList.at(index).customerOrganizationList.at(reportList.at(index).customerOrganizationIndex)});
		sumY += c44.h;

		pdfManager.cursorRowY = sumY;
		pdfManager.CreateRow(5, {{0, reportList.at(index).methodTitle, 8, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Center, FontStyle::Bold}});
		pdfManager.CreateRow(4, {{0, reportList.at(index).technologicalControlMapTitle + " " + reportList.at(index).technologicalControlMap.at(reportList.at(index).technologicalControlMapIndex)}});
		pdfManager.CreateRow(4, {{0, reportList.at(index).normativeDocsTitle + " " + reportList.at(index).normativeDocs}});
		pdfManager.CreateRow(12, {{0, reportList.at(index).equipmentTitle + ": " + reportList.at(index).equipment, 6}});

		pdfManager.CreateRow(4, {{.width = 0, .isRectVisible = false}});
		std::vector<Cell> rowCells = pdfManager.CreateRow(35, {{18, reportList.at(index).weldNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportList.at(index).weldTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportList.at(index).diameterTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {18, reportList.at(index).weldersMarkTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportList.at(index).sectionTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportList.at(index).filmNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {15, reportList.at(index).sensitivityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {30, reportList.at(index).opticalDensityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {15, reportList.at(index).opticalDeltaTitle, 6, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Bottom},
															   {50, reportList.at(index).defectsTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {20, reportList.at(index).acceptableTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
															   {0, reportList.at(index).notesTitle, 6, PoDoFo::PdfHorizontalAlignment::Center}});
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

		pdfManager.CreateRow(4, {{.width = 0, .text = reportList.at(index).extentOfUnacceptableDefectsTitle + ": " + std::format("{:.1f}", reportList.at(index).extentOfUnacceptableDefects), .isRectVisible = false}});
		pdfManager.CreateRow(4, {{.width = 0, .text = reportList.at(index).controlResultTitle + ": " + reportList.at(index).controlResult.at(reportList.at(index).controlResultIndex), .isRectVisible = false}});

		double signHeight = 8;
		pdfManager.cursorRowY = pdfManager.yEnd - signHeight * 4.0;
		pdfManager.CreateRow(signHeight, {{90, reportList.at(index).controllerNameTitle, 8},
										  {40, reportList.at(index).controllerNameList.at(reportList.at(index).controllerNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportList.at(index).controllerOrganization + ", " + reportList.at(index).controllerCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportList.at(index).controlDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportList.at(index).protocolCreateNameTitle, 8},
										  {40, reportList.at(index).protocolCreateNameList.at(reportList.at(index).protocolCreateNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportList.at(index).protocolCreateOrganization + ", " + reportList.at(index).protocolCreateCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportList.at(index).protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportList.at(index).inspectorNameTitle, 7},
										  {40, reportList.at(index).inspectorNameList.at(reportList.at(index).inspectorNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportList.at(index).inspectorOrganization + ", " + reportList.at(index).inspectorCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportList.at(index).protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.CreateRow(signHeight, {{90, reportList.at(index).masterNameTitle, 7},
										  {40, reportList.at(index).masterNameList.at(reportList.at(index).masterNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {100, reportList.at(index).masterOrganization + ", " + reportList.at(index).masterCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
										  {30}, /// подпись
										  {0, reportList.at(index).protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});

		pdfManager.NewPage();

		pdfManager.SaveDocument(std::format("{:s} {:s} от {:s}.pdf", GetCurrentDateString(), reportList.at(index).protocolNumber, reportList.at(index).protocolDate));
	}
}

std::string ReportBuilder::GetCurrentDateString()
{
	auto today = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
	return std::format("{:%d.%m.%Y}", today);
}
