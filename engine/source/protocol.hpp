#pragma once

#include <string>
#include <array>
#include <vector>
#include <cmath>
#include <format>

struct Defect
{
	float width = 0.f;
	float height=0.f;
	int coord=0;
	float length=0.f;
	std::string record;
	std::string coordStr;
};

struct DefRGCData
{
	//RGC
	int nameIndex = 0;
	static inline const std::array<std::string, 18> name{"Aa", "Ak", "Ba", "Ac", "Bc", "Ab", "Bb", "Da", "Dc", "Bd", "Fc2", "E", "Fa", "Fb", "Fe", "Fc1", "∆1", "∆2"};
	int spacerIndex1 = 0;
	int spacerIndex2 = 0;
	static inline const std::array<std::string, 2> spacer{"×", "-"};
	int endIndex = 0;
	static inline const std::array<std::string, 2> end{"≤", ">"};
	float width = 0.f;
	float height = 0.f;
	int coord = 0;
	float length = 0.f;
	
	//UC
	int defNameAmplitudeUCIndex = 0;
	static inline const std::array<std::string, 2> defNameAmplitudeUC{ "Ад", "Ан" };
	int defNameUCIndex = 0;
	static inline const std::array<std::string, 5> defNameUC{ "SH", "LS", "LB", "TD", "CC" };
};

struct ReportData {
	static inline const std::string nameLabTitle{ "Наименование ЛНК" };
	static inline std::string nameLab{ "Лаборатория неразрушающего контроля ООО \"Транснефть - Дальний Восток\" РНУ \"Белогорск\" База производственного обеспечения \"Белогорск\"" };
	static inline const std::string numberAttestationTitle{ "Номер свидетельства об аттестации" };
	static inline std::string numberAttestation{ "XXXXXXXXXX" };

	static inline const std::string protocolNumberTitle{ "ЗАКЛЮЧЕНИЕ №" };
	std::string protocolNumber{ "XXXX-XXXX" };
	std::string controlDate{ "15.07.2026" };
	std::string protocolDate{ "16.07.2026" };

	static inline const std::string objectNameTitle{ "Наименование объекта" };
	std::string objectName{};

	static inline const std::string pipeCategoryTitle = { "Категория трубопровода" };
	int pipeCategoryIndex = 0;
	static inline const std::array<std::string, 6> pipeCategory = { "В", "I", "II", "III", "IV", "-" };

	static inline const std::string contractorOrganizationTitle{ "Подрядная организация" };
	int contractorOrganizationIndex = 0;
	std::vector<std::string> contractorOrganizationList{ "ООО \"Транснефть - Дальний Восток\" РНУ \"Белогорск\" БПО \"Белогорск\" ЛНК" };

	static inline const std::string customerOrganizationTitle{ "Организация заказчика" };
	int customerOrganizationIndex = 0;
	std::vector<std::string> customerOrganizationList{ "ООО \"Транснефть - Дальний Восток\"" };

	std::string methodTitle{ "ПО КОНТРОЛЮ СВАРНЫХ СОЕДИНЕНИЙ РАДИОГРАФИЧЕСКИМ МЕТОДОМ" };

	static inline const std::string technologicalControlMapTitle{ "Контроль выполнен в соответствии с операционной технологической картой" };
	int technologicalControlMapIndex = 0;
	std::vector<std::string> technologicalControlMap = { "ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК" };

	static inline const std::string normativeDocsTitle{ "Оценка качества по" };
	std::string normativeDocs{ "РД-25.160.10-КТН-0016-23 с Изм.1" };

	static inline const std::string equipmentTitle{ "Оборудование и материалы в соответствии с операционной технологической картой" };
	std::string equipment{};

	static inline const std::string weldNumberTitle{ "Номер сварного соединения по журналу сварки" };
	std::string weldNumber{};

	static inline const std::array<std::string, 8> methodList = { "ВИК", "ПВК", "УК", "РК", "ЦРК", "МК", "ПВТ", "Расслоение" };
	int methodIndex{ 0 };

	static inline const std::string weldTypeTitle{ "Тип сварного соединения, способ сварки" };
	static inline const std::vector<std::string> weldType{ "Стыковое", "Стыковое кольцевое", "Угловое" };
	std::vector<std::string> weldingMethod{ "ручная дуговая" };

	static inline const std::string diameterTitle{ "Диаметр, толщина стенки свариваемых элементов, мм" };
	int diameter = 1220;
	float thicknes1 = 0.f;
	float thicknes2 = 0.f;

	static inline const std::string weldersMarkTitle{ "Шифр клейма сварщика/бригады сварщиков" };
	std::vector<std::string> weldersMark1{ "Клеймо сварщика 1" };
	std::vector<std::string> weldersMark2{ "Клеймо сварщика 2" };

	static inline const std::string sectionTypeTitle{ "Тип секций (одношовная или двухшовная). Координаты продольных швов, наименьшее расстояние между продольными швами, мм" };
	static inline const std::array<std::string, 3> sectionType1{ "Одношовная", "Двухшовная", "Бесшовная" };
	static inline const std::array<std::string, 3> sectionType2{ "Одношовная", "Двухшовная", "Бесшовная" };

	static inline const std::string filmNumberTitle{ "Номер снимка, координаты мерного пояса, мм" };
	static inline const std::string sensitivityTitle{ "Чувствительность" };
	float sensitivity = 0.0f;

	float weldOptDenMin = 1.5f;
	int negatoscopeBrightness = 100000;
	float metalOptDenMax;
	std::string opticalDensityTitle{};

	static inline const std::string opticalDeltaTitle{ "Разница оптических плотностей между эталоном чувствительности и основным металлом, е.о.п." };
	float opticalDelta = 0.0f;

	static inline const std::string defectsTitle{ "Описание выявленных дефектов" };

	static inline const std::string acceptableTitle{ "ЗАКЛЮЧЕНИЕ о допустимости выявленных дефектов (допустим/не допустим)" };
	int acceptableIndex = 0;
	static inline const std::vector<std::string> acceptable{ "допустим", "не допустим", "-" };

	static inline const std::string notesTitle{ "Примечания" };

	static inline const std::string extentOfUnacceptableDefectsTitle{ "Суммарная протяжённость недопустимых дефектов, подлежащих устранению с применением сварки, по всей длине/периметру сварного шва, мм" };
	float extentOfUnacceptableDefects = 0.f;

	static inline const std::string controlResultTitle{ "Заключение о годности сварного соединения: («годен», «ремонт», «вырезать», «повторный контроль»)" };
	int controlResultIndex = 0;
	static inline const std::array<std::string, 4> controlResult{ "годен", "ремонт", "вырезать", "повторный контроль" };

	std::string sectionNumber1;
	std::string sectionNumber2;
	int coordSec1Weld1{ 0 };
	int coordSec1Weld2{ 0 };
	int coordSec2Weld1{ 0 };
	int coordSec2Weld2{ 0 };

	int brightness{ 0 };
	int temperature{ 0 };

	static inline const std::array<std::string, 4> roughness{ "Rz20", "Rz40", "Rz60", "Rz80" };
	int roughnessIndex{ 0 };

	float maxHeightOfWeld{ 0.f };
	float minHeightOfWeld{ 0.f };
	float maxWidthOfWeld{ 0.f };
	float minWidthOfWeld{ 0.f };
	float edgeDisplacement{ 0.f };

	static inline const std::string controllerNameTitle{ "Контроль произвёл" };
	int controllerNameIndex{ 0 };
	static inline std::vector<std::string> controllerNameList{ "Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н." };
	std::string controllerOrganization = { "ООО Транснефть - Дальний Восток" };
	std::string controllerCertNumber = { "РСКТН-09050-2021" };

	static inline const std::string protocolCreateNameTitle{ "Заключение выдал" };
	int protocolCreateNameIndex = 0;
	static inline std::vector<std::string> protocolCreateNameList{ "Лопатин А.В.", "Кухаренко И.А.", "Крылов А.Н.", "Федоренко А.Н." };
	std::string protocolCreateOrganization = { "ООО Транснефть - Дальний Восток" };
	std::string protocolCreateCertNumber = { "РСКТН-09050-2021" };

	static inline const std::string inspectorNameTitle{ "Подтвердил полноту проведенного контроля и соответствие оценки качества проконтролированных соединений требованиям НД" };
	int inspectorNameIndex = 0;
	static inline std::vector<std::string> inspectorNameList{ "Караблин Ю.Н." };
	std::string inspectorOrganization = { "ООО Транснефть Надзор" };
	std::string inspectorCertNumber = { "РСКТН-ХХХХХ-ХХХХ" };

	static inline const std::string masterNameTitle{ "Производитель сварочно-монтажных работ с результатами контроля ознакомлен и заключение получил" };
	int masterNameIndex = 0;
	static inline std::vector<std::string> masterNameList{ "Протасевич А.А." };
	std::string masterOrganization = { "ООО Транснефть - Дальний Восток" };
	std::string masterCertNumber = { "ТОР-9АЦ-II-ХХХХ" };
};

class Protocol
{
public:
	explicit Protocol();

	float GetPerimeter(int diam) const;

	/// @brief Максимально допустимая плотность снимка, в зависимости от яркости негатоскопа
	/// @param negBright паспортная яркость негатоскопа
	/// @return Плотность снимка в е.о.п.
	float GetMetalDensity(int negBright);

	/* /// @brief Вычисление количества участков по 300 мм
	/// @param diam диаметр свариваемых труб
	/// @return Количество участков
	int CalculateString(int diam); */
	int CalculateNumString(int numDefects);

	Defect CreateDefectRGC(DefRGCData input);

	std::vector<ReportData>reportList;

	std::vector<DefRGCData> defectDataList;
	std::vector<Defect> defectList;

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