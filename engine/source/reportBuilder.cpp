#include "reportBuilder.hpp"

#include <format>
#include "pdfManager.hpp"
#include "utilities.hpp"

ReportBuilder::ReportBuilder()
{
}

void ReportBuilder::BuildReportRGC(const std::vector<Report> &reportList, const std::vector<int> &reportIndexes)
{
	for (const auto &index : reportIndexes)
	{
		const Report &reportData = reportList.at(index);
		PdfManager pdfManager;
		pdfManager.NewPage();

		double sumY = 0.0;
		double sumX = 0.0;
		Cell c10 = pdfManager.TableCreateCell(0, sumY, 4, {80, reportData.nameLabTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c10.h;
		Cell c20 = pdfManager.TableCreateCell(0, sumY, 16, {80, reportData.nameLab});
		sumY += c20.h;
		Cell c30 = pdfManager.TableCreateCell(0, sumY, 4, {80, reportData.numberAttestationTitle, 8, PoDoFo::PdfHorizontalAlignment::Center});
		sumY += c30.h;
		Cell c40 = pdfManager.TableCreateCell(0, sumY, 4, {80, reportData.numberAttestation});
		sumY += c40.h;

		sumX = c10.w;
		Cell c12 = pdfManager.TableCreateCell(sumX, 0, sumY, {90, reportData.reportNumberTitle + reportData.reportNumber + "\nОт " + NDT::FormatDateForDisplay(reportData.reportDate), 10, PoDoFo::PdfHorizontalAlignment::Center});

		sumX += c12.w;
		sumY = 0.0;
		Cell c13 = pdfManager.TableCreateCell(sumX, sumY, 8, {40, reportData.objectNameTitle});
		sumY += c13.h;
		Cell c23 = pdfManager.TableCreateCell(sumX, sumY, 4, {40, reportData.pipeCategoryTitle});
		sumY += c23.h;
		Cell c33 = pdfManager.TableCreateCell(sumX, sumY, 8, {40, reportData.contractorOrganizationTitle});
		sumY += c33.h;
		Cell c43 = pdfManager.TableCreateCell(sumX, sumY, 8, {40, reportData.customerOrganizationTitle});

		sumX += c43.w;
		sumY = 0.0;
		Cell c14 = pdfManager.TableCreateCell(sumX, sumY, c13.h, {0, reportData.objectName});
		sumY += c14.h;
		Cell c24 = pdfManager.TableCreateCell(sumX, sumY, c23.h, {0, reportData.pipeCategory});
		sumY += c24.h;
		Cell c34 = pdfManager.TableCreateCell(sumX, sumY, c33.h, {0, reportData.contractorOrganization});
		sumY += c34.h;
		Cell c44 = pdfManager.TableCreateCell(sumX, sumY, c43.h, {0, reportData.customerOrganizationList.at(reportData.customerOrganizationIndex)});
		sumY += c44.h;

		pdfManager.cursorRowY = sumY;
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(5, {{0, reportData.methodTitle, 8, PoDoFo::PdfHorizontalAlignment::Center, PoDoFo::PdfVerticalAlignment::Center, FontStyle::Bold}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{0, reportData.technologicalControlMapTitle + " " + reportData.technologicalControlMap.at(reportData.technologicalControlMapIndex)}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{0, reportData.normativeDocsTitle + " " + reportData.normativeDocs}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(12, {{0, reportData.equipmentTitle + ": " + reportData.equipment, 6}});

		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{.width = 0, .isRectVisible = false}});
		pdfManager.TableNewRow();
		std::vector<Cell> rowCells = pdfManager.TableCreateRow(35, {{18, reportData.weldNumberTitle, 6, PoDoFo::PdfHorizontalAlignment::Center},
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
		pdfManager.TableCreateCell(rowCells.at(6).x, rowCells.at(6).y, 4, {rowCells.at(6).w + rowCells.at(7).w + rowCells.at(8).w, "Параметры снимка", 6, PoDoFo::PdfHorizontalAlignment::Center});

		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{rowCells.at(0).w, "1", 8, PoDoFo::PdfHorizontalAlignment::Center},
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
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(0.4, {{0}}); /// двойная граница

		/// Каркас для получения двумерного массива ячеек, для получения координат каждой
		std::vector<std::vector<Cell>> rowCol;
		int num = static_cast<int>(NDT::CalculateNumString(reportData.diameter).size()); /// количество строк в таблице, зависит от диаметра свариваемых труб
		rowCol.reserve(num);
		for (int i = 0; i < num; ++i)
		{
			pdfManager.TableNewRow();
			rowCol.push_back(pdfManager.TableCreateRow(3, {{rowCells.at(0).w, "", 8, PoDoFo::PdfHorizontalAlignment::Center},
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
		}

		pdfManager.cursorRowY -= 3.0 * num;
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(3.0 * num, {{rowCells.at(0).w, reportData.weldNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
											  {rowCells.at(1).w, std::format("{:s}, {:s}", reportData.weldType.at(reportData.weldTypeIndex), reportData.weldingMethod.at(reportData.weldingMethodIndex)), 8, PoDoFo::PdfHorizontalAlignment::Center},
											  {rowCells.at(2).w, std::format("{:d}×{:.1f}×{:.1f}", reportData.diameter, reportData.thicknes1, reportData.thicknes2), 8, PoDoFo::PdfHorizontalAlignment::Center},
											  {rowCells.at(3).w, std::format("{:s}/{:s}", reportData.weldersMark1, reportData.weldersMark2), 8, PoDoFo::PdfHorizontalAlignment::Center}});

		pdfManager.cursorRowY -= 3.0 * num;
		double height = {(3.0 * num) / 4.0};
		pdfManager.TableNewRow(rowCells.at(4).x);
		pdfManager.TableCreateRow(height, {{rowCells.at(4).w / 2, "двухшовная", 6, PoDoFo::PdfHorizontalAlignment::Center},
										   {rowCells.at(4).w / 2, "одношовная", 6, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow(rowCells.at(4).x);
		pdfManager.TableCreateRow(height, {{rowCells.at(4).w / 2, "коорд", 8, PoDoFo::PdfHorizontalAlignment::Center},
										   {rowCells.at(4).w / 2, "коорд", 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow(rowCells.at(4).x);
		pdfManager.TableCreateRow(height, {{rowCells.at(4).w / 2, "коорд", 8, PoDoFo::PdfHorizontalAlignment::Center},
										   {rowCells.at(4).w / 2, "коорд", 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow(rowCells.at(4).x);
		pdfManager.TableCreateRow(height, {{rowCells.at(4).w, "минимум", 8, PoDoFo::PdfHorizontalAlignment::Center}});

		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{.width = 0, .text = reportData.extentOfUnacceptableDefectsTitle + ": " + std::format("{:.1f}", reportData.extentOfUnacceptableDefects), .isRectVisible = false}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(4, {{.width = 0, .text = reportData.controlResultTitle + ": " + reportData.controlResult.at(reportData.controlResultIndex), .isRectVisible = false}});

		double signHeight = 8.0;
		pdfManager.cursorRowY = {pdfManager.yEnd - signHeight * 4.0};
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(signHeight, {{90, reportData.controllerNameTitle, 8},
											   {40, reportData.controllerName, 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {100, reportData.controllerOrganization + ", " + reportData.controllerCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {30}, /// подпись
											   {0, NDT::FormatDateForDisplay(reportData.controlDate), 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(signHeight, {{90, reportData.protocolCreateNameTitle, 8},
											   {40, reportData.protocolCreateNameList.at(reportData.protocolCreateNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {100, reportData.protocolCreateOrganization + ", " + reportData.protocolCreateCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {30}, /// подпись
											   {0, NDT::FormatDateForDisplay(reportData.reportDate), 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(signHeight, {{90, reportData.inspectorNameTitle, 7},
											   {40, reportData.inspectorNameList.at(reportData.inspectorNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {100, reportData.inspectorOrganization + ", " + reportData.inspectorCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {30}, /// подпись
											   {0, NDT::FormatDateForDisplay(reportData.reportDate), 8, PoDoFo::PdfHorizontalAlignment::Center}});
		pdfManager.TableNewRow();
		pdfManager.TableCreateRow(signHeight, {{90, reportData.masterNameTitle, 7},
											   {40, reportData.masterNameList.at(reportData.masterNameIndex), 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {100, reportData.masterOrganization + ", " + reportData.masterCertNumber, 8, PoDoFo::PdfHorizontalAlignment::Center},
											   {30}, /// подпись
											   {0, NDT::FormatDateForDisplay(reportData.reportDate), 8, PoDoFo::PdfHorizontalAlignment::Center}});

		pdfManager.NewPage();

		pdfManager.SaveDocument(std::format("{:s} {:s} от {:s}.pdf", NDT::GetCurrentIsoDate(), reportData.reportNumber, reportData.reportDate));
	}
}
