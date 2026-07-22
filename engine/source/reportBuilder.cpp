#include "reportBuilder.hpp"
#include <format>
#include "protocol.hpp"
#include "pdfManager.hpp"

ReportBuilder::ReportBuilder(Protocol &protocol)
	: protocol{protocol}
{
}

void ReportBuilder::BuildReportRGC()
{
	PdfManager pdfManager;
	
	pdfManager.NewPage();

	double sumY = 0.0;
	double sumX = 0.0;
	Cell c10 = pdfManager.CreateCell(0, sumY, 4, {80, protocol.nameLabTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
	sumY += c10.h;
	Cell c20 = pdfManager.CreateCell(0, sumY, 16, {80, protocol.nameLab});
	sumY += c20.h;
	Cell c30 = pdfManager.CreateCell(0, sumY, 4, {80, protocol.numberAttestationTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
	sumY += c30.h;
	Cell c40 = pdfManager.CreateCell(0, sumY, 4, {80, protocol.numberAttestation});
	sumY += c40.h;

	sumX = c10.w;
	Cell c12 = pdfManager.CreateCell(sumX, 0, sumY, {90, protocol.protocolNumberTitle + protocol.protocolNumber + "\nОт " + protocol.protocolDate, 10, PoDoFo::PdfHorizontalAlignment::Center});

	sumX += c12.w;
	sumY = 0.0;
	Cell c13 = pdfManager.CreateCell(sumX, sumY, 8, {40, protocol.objectNameTitle});
	sumY += c13.h;
	Cell c23 = pdfManager.CreateCell(sumX, sumY, 4, {40, protocol.pipeCategoryTitle});
	sumY += c23.h;
	Cell c33 = pdfManager.CreateCell(sumX, sumY, 8, {40, protocol.contractorOrganizationTitle});
	sumY += c33.h;
	Cell c43 = pdfManager.CreateCell(sumX, sumY, 8, {40, protocol.customerOrganizationTitle});

	sumX += c43.w;
	sumY = 0.0;
	Cell c14 = pdfManager.CreateCell(sumX, sumY, c13.h, {0, protocol.objectName});
	sumY += c14.h;
	Cell c24 = pdfManager.CreateCell(sumX, sumY, c23.h, {0, protocol.pipeCategory.at(protocol.pipeCategoryIndex)});
	sumY += c24.h;
	Cell c34 = pdfManager.CreateCell(sumX, sumY, c33.h, {0, protocol.contractorOrganizationList.at(protocol.contractorOrganizationIndex)});
	sumY += c34.h;
	Cell c44 = pdfManager.CreateCell(sumX, sumY, c43.h, {0, protocol.customerOrganizationList.at(protocol.customerOrganizationIndex)});
	sumY += c44.h;

	pdfManager.cursorRowY = sumY;
	pdfManager.CreateRow(5, {{0, protocol.methodTitle, 8, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Center, FontStyle::Bold}});
	pdfManager.CreateRow(4, {{0, protocol.technologicalControlMapTitle + " " + protocol.technologicalControlMap.at(protocol.technologicalControlMapIndex)}});
	pdfManager.CreateRow(4, {{0, protocol.normativeDocsTitle + " " + protocol.normativeDocs}});
	pdfManager.CreateRow(12, {{0, protocol.equipmentTitle + ": " + protocol.equipment, 6}});

	pdfManager.CreateRow(4, {{.width = 0, .isRectVisible = false}});
	std::vector<Cell> rowCells = pdfManager.CreateRow(35, {{18, protocol.weldNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, protocol.weldTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, protocol.diameterTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {18, protocol.weldersMarkTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, protocol.sectionTypeTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, protocol.filmNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {15, protocol.sensitivityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {30, protocol.opticalDensityTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {15, protocol.opticalDeltaTitle, 6, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Bottom},
														   {50, protocol.defectsTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {20, protocol.acceptableTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
														   {0, protocol.notesTitle, 6, PoDoFo::PdfHorizontalAlignment::Center}});
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

	pdfManager.CreateRow(4, {{.width = 0, .text = protocol.extentOfUnacceptableDefectsTitle + ": " + std::format("{:.1f}", protocol.extentOfUnacceptableDefects), .isRectVisible = false}});
	pdfManager.CreateRow(4, {{.width = 0, .text = protocol.controlResultTitle + ": " + protocol.controlResult.at(protocol.controlResultIndex), .isRectVisible = false}});

	double signHeight = 8;
	pdfManager.cursorRowY = pdfManager.yEnd - signHeight * 4.0;
	pdfManager.CreateRow(signHeight, {{90, protocol.controllerNameTitle, 8},
									  {40, protocol.controllerNameList.at(protocol.controllerNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, protocol.controllerOrganization + ", " + protocol.controllerCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, protocol.controlDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, protocol.protocolCreateNameTitle, 8},
									  {40, protocol.protocolCreateNameList.at(protocol.protocolCreateNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, protocol.protocolCreateOrganization + ", " + protocol.protocolCreateCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, protocol.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, protocol.inspectorNameTitle, 7},
									  {40, protocol.inspectorNameList.at(protocol.inspectorNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, protocol.inspectorOrganization + ", " + protocol.inspectorCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, protocol.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});
	pdfManager.CreateRow(signHeight, {{90, protocol.masterNameTitle, 7},
									  {40, protocol.masterNameList.at(protocol.masterNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {100, protocol.masterOrganization + ", " + protocol.masterCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
									  {30}, /// подпись
									  {0, protocol.protocolDate, 8, PoDoFo::PdfHorizontalAlignment::Center}});

	pdfManager.NewPage();

	pdfManager.SaveDocument();
}
