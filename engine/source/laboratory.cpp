#include "laboratory.hpp"

#include "applicationData.hpp"

Laboratory::Laboratory(ApplicationData &appData)
{
	dbManager = std::make_unique<DatabaseManager>(appData.pathToDatabase);

	// metalOptDenMax = static_cast<float>(std::log10(negatoscopeBrightness) - 2.0f);
	// opticalDensityTitle = std::format("Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
	// weldOptDenMin, metalOptDenMax, negatoscopeBrightness);
}

void Laboratory::LoadDB()
{
	dbManager->LoadLaboratoryInfo(*this);
	employeesList = dbManager->LoadEmployees();
	inspectorsList = dbManager->LoadInspectors();
	mastersList = dbManager->LoadMasters();
	weldersList = dbManager->LoadWelders();
	equpmentsList = dbManager->LoadEquipment();
	controlMapsList = dbManager->LoadControlMaps();
	normativeDocumentsList = dbManager->LoadNormativeDocuments();
}

void Laboratory::SaveDB()
{
	dbManager->SaveLaboratoryInfo(*this);
	dbManager->SaveEmployees(employeesList);
	dbManager->SaveInspectors(inspectorsList);
	dbManager->SaveMasters(mastersList);
	dbManager->SaveWelders(weldersList);
	dbManager->SaveEquipment(equpmentsList);
	dbManager->SaveControlMaps(controlMapsList);
	dbManager->SaveNormativeDocuments(normativeDocumentsList);
}
