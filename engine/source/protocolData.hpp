#pragma once

#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <format>

class ProtocolData
{
public:
	uint GetPerimeter(uint diam);
	float GetMetalDensity(uint negBright);

	std::string nameLabTitle{"Наименование ЛНК"};
	std::string nameLab{"Лаборатория неразрушающего контроля БПО Белогорск"};
	std::string numberAttestationTitle{"Номер свидетельства об аттестации"};
	std::string numberAttestation{"XXXXXXXXXX"};

	std::string protocolNumberTitle{"ЗАКЛЮЧЕНИЕ №"};
	std::string protocolNumber{"XXXX-XXXX"};
	std::string controlDate{"15.07.2026"};
	std::string protocolDate{"15.07.2026"};

	std::string objectNameTitle{"Наименование объекта"};
	std::string objectName{};

	std::string pipeCategoryTitle = {"Категория трубопровода"};
	uint pipeCategoryIndex = {0};
	std::array<std::string, 6> pipeCategory = {"В", "I", "II", "III", "IV", "-"};

	std::string contractorOrganizationTitle{"Подрядная организация"};
	uint contractorOrganizationIndex = 0;
	std::vector<std::string> contractorOrganizationList{""};

	std::string customerOrganizationTitle{"Организация заказчика"};
	uint customerOrganizationIndex = 0;
	std::vector<std::string> customerOrganizationList{""};

	std::string methodTitle{"ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ"};

	std::string technologicalControlMapTitle{"Контроль выполнен в соответствии с операционной технологической картой"};
	uint technologicalControlMapIndex{0};
	std::vector<std::string> technologicalControlMap = {"ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК"};

	std::string normativeDocsTitle{"Оценка качества по"};
	std::string normativeDocs{"РД-25.160.10-КТН-0016-23 с Изм.1"};

	std::string equipmentTitle{"Оборудование и материалы в соответствии с операционной технологической картой"};
	std::string equipment{""};

	std::string weldNumberTitle{"Номер сварного соединения по журналу сварки"};
	std::string weldNumber{};

	std::array<std::string, 8> methodList = {"ВИК", "ПВК", "УК", "РК", "ЦРК", "МК", "ПВТ", "Расслоение"};
	uint methodIndex{0};

	std::string weldTypeTitle{"Тип сварного соединения, способ сварки"};
	std::vector<std::string> weldType{"Стыковое", "Стыковое кольцевое", "Угловое"};
	std::vector<std::string> weldingMethod{"ручная дуговая"};

	std::string diameterTitle{"Диаметр, толщина стенки свариваемых элементов, мм"};
	uint diameter = 0;
	float thicknes1 = 0.f;
	float thicknes2 = 0.f;

	std::string weldersMarkTitle{"Шифр клейма сварщика/бригады сварщиков"};
	std::vector<std::string> weldersMark1{"Клеймо сварщика 1"};
	std::vector<std::string> weldersMark2{"Клеймо сварщика 2"};

	std::string sectionTypeTitle{"Тип секций (одношовная или двухшовная). Координаты продольных швов, наименьшее расстояние между продольными швами, мм"};
	std::array<std::string, 3> sectionType1{"Одношовная", "Двухшовная", "Бесшовная"};
	std::array<std::string, 3> sectionType2{"Одношовная", "Двухшовная", "Бесшовная"};

	std::string filmNumberTitle{"Номер снимка, координаты мерного пояса, мм"};
	std::string sensitivityTitle{"Чувствительность"};
	float sensitivity = 0.0f;

	float weldOptDenMin = 1.5f;
	uint negatoscopeBrightness = 100000;
	float metalOptDenMax = std::log10(negatoscopeBrightness) - 2.0f;
	std::string opticalDensityTitle{std::format(
		"Оптическая плотность самого светлого участка сварного шва, не менее {:.1f} е.о.п./ наибольшая оптическая плотность основного металла в зоне контроля, {:.1f} е.о.п/фактическая яркость негатоскопа {:d} кд/м2",
		weldOptDenMin, metalOptDenMax, negatoscopeBrightness)};

	std::string sectionNumber1;
	std::string sectionNumber2;
	uint coordSec1Weld1{0};
	uint coordSec1Weld2{0};
	uint coordSec2Weld1{0};
	uint coordSec2Weld2{0};

	uint brightness{0};
	int temperature{0};

	std::array<std::string, 4> roughness{"Rz20", "Rz40", "Rz60", "Rz80"};
	uint roughnessIndex{0};

	// std::string stringOfDefects{""};
	std::vector<std::string> acceptable{"допустим", "не допустим", "-"};
	uint acceptableIndex{0};

	float extentOfUnacceptableDefects{0.f}; // Суммарная протяжённость недопуст. дефектов
	float maxHeightOfWeld{0.f};
	float minHeightOfWeld{0.f};
	float maxWidthOfWeld{0.f};
	float minWidthOfWeld{0.f};
	float edgeDisplacement{0.f};

	std::string controllerNameTitle{"Контроль произвёл"};
	uint controllerNameIndex{0};
	std::vector<std::string> controllerNameList{"Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н."};
	std::string controllerOrganization = {};
	std::string controllerCertNumber = {};

	std::string protocolCreateNameTitle{"Заключение выдал"};
	uint protocolCreateNameIndex{0};
	std::vector<std::string> protocolCreateNameList{"Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н."};
	std::string protocolCreateOrganizationName = {};
	std::string protocolCreatoeCertNumber = {};

	std::string inspectorNameTitle{"Подтвердил полноту проведенного контроля и соответствие оценки качества проконтролированных соединений требованиям НД"};
	uint inspectorNameIndex{0};
	std::vector<std::string> inspectorNameList{"Караблин Ю.Н."};
	std::string inspectorOrganization = {};
	std::string inspectorCertNumber = {};

	std::string masterNameTitle{"Производитель сварочно-монтажных работ с результатами контроля ознакомлен и заключение получил"};
	uint masterNameIndex{0};
	std::vector<std::string> masterNameList{"Протасевич А.А."};
	std::string masterOrganization = {};
	std::string masterCertNumber = {};

	std::array<std::string, 4> controlResult{"годен", "ремонт", "вырезать", "повторный контроль"};
};