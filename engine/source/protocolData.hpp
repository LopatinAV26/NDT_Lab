#pragma once

#include <string>
#include <array>
#include <vector>

class ProtocolData
{
public:
	std::string nameLabTitle{ "Наименование ЛНК" };
	std::string nameLab{ "Лаборатория неразрушающего контроля БПО Белогорск" };
	std::string numberAttestationTitle{ "Номер свидетельства об аттестации" };
	std::string numberAttestation{ "XXXXXXXXXX" };

	std::string protocolNumberTitle{ "ЗАКЛЮЧЕНИЕ №" };
	std::string protocolNumber{ "XXXX-XXXX" };
	std::string controlDate{ "15.07.2026" };
	std::string protocolDate{ "15.07.2026" };

	std::string objectNameTitle{ "Наименование объекта" };
	std::string objectName{};

	std::string pipeCategoryTitle = { "Категория трубопровода" };
	int pipeCategoryIndex = { 0 };
	std::array<std::string, 6> pipeCategory = { "В", "I", "II", "III", "IV", "-" };

	std::string contractorOrganizationTitle{ "Подрядная организация" };
	int contractorOrganizationIndex = 0;
	std::vector<std::string> contractorOrganizationList{ "" };

	std::string customerOrganizationTitle{ "Организация заказчика" };
	int customerOrganizationIndex = 0;
	std::vector<std::string> customerOrganizationList{ "" };

	std::string methodTitle{ "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ" };

	std::string technologicalControlMapTitle{ "Контроль выполнен в соответствии с операционной технологической картой" };
	int technologicalControlMapIndex{ 0 };
	std::vector<std::string> technologicalControlMap = { "ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК" };

	std::string normativeDocsTitle{ "Оценка качества по" };
	std::string normativeDocs{ "РД-25.160.10-КТН-0016-23 с Изм.1" };

	std::string equipmentTitle{ "Оборудование и материалы в соответствии с операционной технологической картой" };
	std::string equipment{ "" };



	std::string weldNumber{ "Номер сварного соединения" };

	std::array<std::string, 8> methodList = { "ВИК", "ПВК", "УК", "РК", "ЦРК", "МК", "ПВТ", "Расслоение" };
	int methodIndex{ 0 };

	

	std::vector<std::string> weldType{ "Стыковое", "Стыковое кольцевое", "Угловое" };
	std::vector<std::string> weldingMethod{ "ручная дуговая" };
	float diameter{ 0.f };
	float thicknes1{ 0.f };
	float thicknes2{ 0.f };
	float perimeter{ 0.f };
	std::array<std::string, 3> sectionType1{ "Одношовная", "Двухшовная", "Бесшовная" };
	std::array<std::string, 3> sectionType2{ "Одношовная", "Двухшовная", "Бесшовная" };
	std::string sectionNumber1;
	std::string sectionNumber2;
	int coordSec1Weld1{ 0 };
	int coordSec1Weld2{ 0 };
	int coordSec2Weld1{ 0 };
	int coordSec2Weld2{ 0 };
	std::vector<std::string> weldersMark1{ "Клеймо сварщика 1" };
	std::vector<std::string> weldersMark2{ "Клеймо сварщика 2" };
	int brightness{ 0 };
	int temperature{ 0 };

	std::array<std::string, 4> roughness{ "Rz20", "Rz40", "Rz60", "Rz80" };
	int roughnessIndex{ 0 };

	// std::string stringOfDefects{""};
	std::vector<std::string> acceptable{ "допустим", "не допустим", "-" };
	int acceptableIndex{ 0 };

	float extentOfUnacceptableDefects{ 0.f }; // Суммарная протяжённость недопуст. дефектов
	float maxHeightOfWeld{ 0.f };
	float minHeightOfWeld{ 0.f };
	float maxWidthOfWeld{ 0.f };
	float minWidthOfWeld{ 0.f };
	float edgeDisplacement{ 0.f };

	std::string controllerNameTitle{ "Контроль произвёл" };
	int controllerNameIndex{ 0 };
	std::vector<std::string> controllerNameList{ "Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н." };
	std::string controllerOrganization = {};
	std::string controllerCertNumber = {};

	std::string protocolCreateNameTitle{ "Заключение выдал" };
	int protocolCreateNameIndex{ 0 };
	std::vector<std::string> protocolCreateNameList{ "Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н." };
	std::string protocolCreateOrganizationName = {};
	std::string protocolCreatoeCertNumber = {};

	std::string inspectorNameTitle{ "Подтвердил полноту проведенного контроля и соответствие оценки качества проконтролированных соединений требованиям НД" };
	int inspectorNameIndex{ 0 };
	std::vector<std::string> inspectorNameList{ "Караблин Ю.Н." };
	std::string inspectorOrganization = {};
	std::string inspectorCertNumber = {};

	std::string masterNameTitle{ "Производитель сварочно-монтажных работ с результатами контроля ознакомлен и заключение получил" };
	int masterNameIndex{ 0 };
	std::vector<std::string> masterNameList{ "Протасевич А.А." };
	std::string masterOrganization = {};
	std::string masterCertNumber = {};

	std::array<std::string, 4> controlResult{ "годен", "ремонт", "вырезать", "повторный контроль" };
};