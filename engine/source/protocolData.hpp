#pragma once

#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <format>

class ProtocolData
{
public:
	ProtocolData();

	float GetPerimeter(int diam);

	/// @brief Максимально допустимая плотность снимка, в зависимости от яркости негатоскопа
	/// @param negBright паспортная яркость негатоскопа
	/// @return Плотность снимка в е.о.п.
	float GetMetalDensity(int negBright);

	/// @brief Вычисление количества участков по 300 мм
	/// @param diam диаметр свариваемых труб
	/// @return Количество участков
	int CalculateString(int diam);

	std::string nameLabTitle{"Наименование ЛНК"};
	std::string nameLab{"Лаборатория неразрушающего контроля ООО \"Транснефть - Дальний Восток\" РНУ \"Белогорск\" База производственного обеспечения \"Белогорск\""};
	std::string numberAttestationTitle{"Номер свидетельства об аттестации"};
	std::string numberAttestation{"XXXXXXXXXX"};

	std::string protocolNumberTitle{"ЗАКЛЮЧЕНИЕ №"};
	std::string protocolNumber{"XXXX-XXXX"};
	std::string controlDate{"15.07.2026"};
	std::string protocolDate{"16.07.2026"};

	std::string objectNameTitle{"Наименование объекта"};
	std::string objectName{};

	std::string pipeCategoryTitle = {"Категория трубопровода"};
	int pipeCategoryIndex = 0;
	std::array<std::string, 6> pipeCategory = {"В", "I", "II", "III", "IV", "-"};

	std::string contractorOrganizationTitle{"Подрядная организация"};
	int contractorOrganizationIndex = 0;
	std::vector<std::string> contractorOrganizationList{"ООО \"Транснефть - Дальний Восток\" РНУ \"Белогорск\" БПО \"Белогорск\" ЛНК"};

	std::string customerOrganizationTitle{"Организация заказчика"};
	int customerOrganizationIndex = 0;
	std::vector<std::string> customerOrganizationList{"ООО \"Транснефть - Дальний Восток\""};

	std::string methodTitle{"ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ"};

	std::string technologicalControlMapTitle{"Контроль выполнен в соответствии с операционной технологической картой"};
	int technologicalControlMapIndex = 0;
	std::vector<std::string> technologicalControlMap = {"ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК"};

	std::string normativeDocsTitle{"Оценка качества по"};
	std::string normativeDocs{"РД-25.160.10-КТН-0016-23 с Изм.1"};

	std::string equipmentTitle{"Оборудование и материалы в соответствии с операционной технологической картой"};
	std::string equipment{};

	std::string weldNumberTitle{"Номер сварного соединения по журналу сварки"};
	std::string weldNumber{};

	std::array<std::string, 8> methodList = {"ВИК", "ПВК", "УК", "РК", "ЦРК", "МК", "ПВТ", "Расслоение"};
	int methodIndex{0};

	std::string weldTypeTitle{"Тип сварного соединения, способ сварки"};
	std::vector<std::string> weldType{"Стыковое", "Стыковое кольцевое", "Угловое"};
	std::vector<std::string> weldingMethod{"ручная дуговая"};

	std::string diameterTitle{"Диаметр, толщина стенки свариваемых элементов, мм"};
	int diameter = 1220;
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
	int negatoscopeBrightness = 100000;
	float metalOptDenMax;
	std::string opticalDensityTitle{};

	std::string opticalDeltaTitle{"Разница оптических плотностей между эталоном чувствительности и основным металлом, е.о.п."};
	float opticalDelta = 0.0f;

	std::string defectsTitle{"Описание выявленных дефектов"};

	std::string acceptableTitle{"ЗАКЛЮЧЕНИЕ о допустимости выявленных дефектов (допустим/не допустим)"};
	int acceptableIndex = 0;
	std::vector<std::string> acceptable{"допустим", "не допустим", "-"};

	std::string notesTitle{"Примечания"};

	std::string extentOfUnacceptableDefectsTitle{"Суммарная протяжённость недопустимых дефектов, подлежащих устранению с применением сварки, по всей длине/периметру сварного шва, мм"};
	float extentOfUnacceptableDefects = 0.f;

	std::string controlResultTitle{"Заключение о годности сварного соединения: («годен», «ремонт», «вырезать», «повторный контроль»)"};
	int controlResultIndex = 0;
	std::array<std::string, 4> controlResult{"годен", "ремонт", "вырезать", "повторный контроль"};

	std::string sectionNumber1;
	std::string sectionNumber2;
	int coordSec1Weld1{0};
	int coordSec1Weld2{0};
	int coordSec2Weld1{0};
	int coordSec2Weld2{0};

	int brightness{0};
	int temperature{0};

	std::array<std::string, 4> roughness{"Rz20", "Rz40", "Rz60", "Rz80"};
	int roughnessIndex{0};

	float maxHeightOfWeld{0.f};
	float minHeightOfWeld{0.f};
	float maxWidthOfWeld{0.f};
	float minWidthOfWeld{0.f};
	float edgeDisplacement{0.f};

	std::string controllerNameTitle{"Контроль произвёл"};
	int controllerNameIndex{0};
	std::vector<std::string> controllerNameList{"Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н."};
	std::string controllerOrganization = {"ООО Транснефть - Дальний Восток"};
	std::string controllerCertNumber = {"РСКТН-09050-2021"};

	std::string protocolCreateNameTitle{"Заключение выдал"};
	int protocolCreateNameIndex = 0;
	std::vector<std::string> protocolCreateNameList{"Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н."};
	std::string protocolCreateOrganization = {"ООО Транснефть - Дальний Восток"};
	std::string protocolCreateCertNumber = {"РСКТН-09050-2021"};

	std::string inspectorNameTitle{"Подтвердил полноту проведенного контроля и соответствие оценки качества проконтролированных соединений требованиям НД"};
	int inspectorNameIndex = 0;
	std::vector<std::string> inspectorNameList{"Караблин Ю.Н."};
	std::string inspectorOrganization = {"ООО Транснефть Надзор"};
	std::string inspectorCertNumber = {"РСКТН-ХХХХХ-ХХХХ"};

	std::string masterNameTitle{"Производитель сварочно-монтажных работ с результатами контроля ознакомлен и заключение получил"};
	int masterNameIndex = 0;
	std::vector<std::string> masterNameList{"Протасевич А.А."};
	std::string masterOrganization = {"ООО Транснефть - Дальний Восток"};
	std::string masterCertNumber = {"ТОР-9АЦ-II-ХХХХ"};
};