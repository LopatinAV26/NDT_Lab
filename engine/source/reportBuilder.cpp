#include "reportBuilder.hpp"
#include <format>
#include "applicationData.hpp"
#include "protocolData.hpp"
#include "pdfManager.hpp"

ReportBuilder::ReportBuilder(ProtocolData &protocolData)
	: data{protocolData}
{
}

void ReportBuilder::CreateReportRGC(ApplicationData &coreAppData)
{
	PdfManager pdfManager(coreAppData);
	pdfManager.NewPage();

	double sumY = 0.0;
	double sumX = 0.0;
	Cell c10 = pdfManager.CreateCell(0, sumY, 4, {80, data.nameLabTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
	sumY += c10.h;
	Cell c20 = pdfManager.CreateCell(0, sumY, 16, {80, data.nameLab});
	sumY += c20.h;
	Cell c30 = pdfManager.CreateCell(0, sumY, 4, {80, data.numberAttestationTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
	sumY += c30.h;
	Cell c40 = pdfManager.CreateCell(0, sumY, 4, {80, data.numberAttestation});
	sumY += c40.h;

	sumX = c10.w;
	Cell c12 = pdfManager.CreateCell(sumX, 0, sumY, {90, data.protocolNumberTitle + data.protocolNumber + "\nОт " + data.protocolDate, 10, PoDoFo::PdfHorizontalAlignment::Center});

	sumX += c12.w;
	sumY = 0.0;
	Cell c13 = pdfManager.CreateCell(sumX, sumY, 8, {40, data.objectNameTitle});
	sumY += c13.h;
	Cell c23 = pdfManager.CreateCell(sumX, sumY, 4, {40, data.pipeCategoryTitle});
	sumY += c23.h;
	Cell c33 = pdfManager.CreateCell(sumX, sumY, 8, {40, data.contractorOrganizationTitle});
	sumY += c33.h;
	Cell c43 = pdfManager.CreateCell(sumX, sumY, 8, {40, data.customerOrganizationTitle});

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
	pdfManager.CreateRow(5, {{0, data.methodTitle, 8, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Center, FontStyle::Bold}});
	pdfManager.CreateRow(4, {{0, data.technologicalControlMapTitle + " " + data.technologicalControlMap.at(data.technologicalControlMapIndex)}});
	pdfManager.CreateRow(4, {{0, data.normativeDocsTitle + " " + data.normativeDocs}});
	pdfManager.CreateRow(12, {{0, data.equipmentTitle + ": " + data.equipment, 6}});

	pdfManager.CreateRow(4, {{.width = 0, .isRectVisible = false}});
	std::vector<Cell> rowCells = pdfManager.CreateRow(35, {{18, data.weldNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, data.weldTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, data.diameterTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, data.weldersMarkTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, data.sectionTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, data.filmNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {15, data.sensitivityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {30, data.opticalDensityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {15, data.opticalDeltaTitle, 6, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Bottom},
														   {50, data.defectsTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, data.acceptableTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {0, data.notesTitle, 6, PoDoFo::PdfHorizontalAlignment::Center}});
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

	/// Каркас для получения двумерного массива ячеек, для получения координат каждой
	/* std::vector<std::vector<Cell>> rowCol;
	int num = data.CalculateString(data.diameter); /// количество строк в таблице, зависит от диаметра свариваемых труб
	rowCol.reserve(num);
	for (int i = 0; i < num; ++i)
	{
		rowCol.push_back(pdfManager.CreateRow(3, {{rowCells.at(0).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(1).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(2).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(3).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(4).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(5).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(6).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(7).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(8).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(9).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(10).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
												  {rowCells.at(11).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center}}));
	} */
	///////////////////////////////////////
	// нарисовать таблицу переменной структуры
	/////////////////////////////////////

	pdfManager.CreateRow(4, {{.width = 0, .text = data.extentOfUnacceptableDefectsTitle + ": " + std::format("{:.1f}", data.extentOfUnacceptableDefects), .isRectVisible = false}});
	pdfManager.CreateRow(4, {{.width = 0, .text = data.controlResultTitle + ": " + data.controlResult.at(data.controlResultIndex), .isRectVisible = false}});

	double signHeight = 8;
	pdfManager.cursorRowY = pdfManager.yEnd - signHeight * 4.0;
	pdfManager.CreateRow(signHeight, {{90, data.controllerNameTitle, 8},
									  {40, data.controllerNameList.at(data.controllerNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, data.controllerOrganization + ", " + data.controllerCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, data.controlDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, data.protocolCreateNameTitle, 8},
									  {40, data.protocolCreateNameList.at(data.protocolCreateNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, data.protocolCreateOrganization + ", " + data.protocolCreateCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, data.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, data.inspectorNameTitle, 7},
									  {40, data.inspectorNameList.at(data.inspectorNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, data.inspectorOrganization + ", " + data.inspectorCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, data.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, data.masterNameTitle, 7},
									  {40, data.masterNameList.at(data.masterNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, data.masterOrganization + ", " + data.masterCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, data.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});

	pdfManager.NewPage();

	pdfManager.SaveDocument();
}
