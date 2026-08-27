#pragma once

#include <string>
#include <format>
#include <array>
#include <vector>
#include <chrono>

#include "utilities.hpp"

enum class DefectRT : uint8_t
{
    Aa,     /// единичная сферическая и удлиннённая пора
    Ak,     /// канальная пора
    Ba,     /// единичное компактное шлаковое включение
    Ac,     /// скопление пор
    Bc,     /// скопление шлаковых включений
    Ab,     /// цепочка пор
    Bb,     /// цепочка шлаковых включений
    Da,     /// непровар
    Dc,     /// несплавление
    Bd,     /// удлинённый зашлакованный карман
    Fc2,    /// внутренний подрез
    E,      /// трещина
    Fa,     /// вогнутость корня
    Fb,     /// превышение проплава
    Fe,     /// дефект сборки, шлифовка околошовной зоны
    delta1, /// чешуйчатость
    delta2, /// западание между валиками
    Fc1,    /// наружный подрез
    Fd,     /// смещение кромок
    Mw      /// металлическое включение
};

struct DefRT
{
    std::string id = NDT::GenerateUuidV7();
    std::chrono::sys_seconds updatedAt =
        std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    std::optional<std::chrono::sys_seconds> deletedAt;

    int nameIndex = 0;
    int endIndex = 0;
    float length = 0.f; ///< протяжённость, для скоплений и цепочек
    float width = 0.f;
    float height = 0.f;
    int coord = 0;
    std::string record;
    std::string coordStr;
    static inline const std::array<std::string, 20> name{"Aa", "Ak", "Ba", "Ac", "Bc", "Ab", "Bb", "Da", "Dc", "Bd", "Fc2", "E", "Fa", "Fb", "Fe", "Fc1", "∆1", "∆2", "Fd", "Mw"};
    static inline const std::array<std::string, 2> end{"≤", ">"};
};

struct DefUC
{
    int defNameAmplitudeUCIndex = 0;
    int defNameUCIndex = 0;
    static inline const std::array<std::string, 2> defNameAmplitudeUC{"Ад", "Ан"};
    static inline const std::array<std::string, 5> defNameUC{"SH", "LS", "LB", "TD", "CC"};
};

enum class Method : uint8_t
{
    VT,  /// визуальный и измерительный контроль
    RT,  /// радиографический контроль
    DRT, /// цифровой радиографический контроль
    UT,  /// ультразвуковой контроль
    PT,  /// капиллярный контроль
    LT,  /// течеискание
    MT,  /// магнитопорошковый контроль
    ECT, /// Eddy Current Testing - вихретоковый контроль
    DT,  /// Delamination - расслоение
    UTM  /// Ultrasonic Thickness Measurement - ультразвуковое измерение толщины
};

class Report
{
public:
    Report();

    /// @brief Получить заголовок отчёта
    /// @param name
    /// @return
    std::string GetMethodTitle(Method value) const;
    std::string GetDefectRTName(DefectRT value) const;

    std::string id = NDT::GenerateUuidV7(); // UUID v7, генерируется на клиенте при создании записи
    std::chrono::sys_seconds updatedAt =
        std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()); // когда запись последний раз менялась
    std::optional<std::chrono::sys_seconds> deletedAt;                              // nullopt = не удалена; иначе - момент "мягкого" удаления

    Method methodValue;
    std::string method;

    std::string controlDate; ///< дата НК
    std::string reportDate;  ///< дата выдачи заключения

    std::string nameLabTitle{"Наименование ЛНК"};
    std::string nameLab;
    std::string numberAttestationTitle{"Номер свидетельства об аттестации"};
    std::string numberAttestation;

    std::string weldNumberTitle{"Номер сварного соединения по журналу сварки"};
    std::string weldNumber;

    std::string reportNumberTitle{"ЗАКЛЮЧЕНИЕ №"};
    std::string reportNumber;

    std::string objectNameTitle{"Наименование объекта"};
    std::string objectName;

    std::array<std::string, 5> pipeCategoryList = {"В", "I", "II", "III", "IV"};
    std::string pipeCategoryTitle{"Категория трубопровода"};
    std::string pipeCategory;

    std::string contractorOrganizationTitle{"Подрядная организация"};
    std::string contractorOrganization;

    std::string customerOrganizationTitle{"Организация заказчика"};
    std::string customerOrganization;

    std::string controllerNameTitle{"Контроль произвёл"};
    std::string controllerName;
    std::string controllerOrganization;
    std::string controllerCertNumber;

    std::string protocolCreateNameTitle{"Заключение выдал"};
    std::string protocolCreateName;
    std::string protocolCreateOrganization;
    std::string protocolCreateCertNumber;

    std::string inspectorNameTitle{"Подтвердил полноту проведенного контроля и соответствие оценки качества проконтролированных соединений требованиям НД"};
    std::string inspectorName;
    std::string inspectorOrganization;
    std::string inspectorCertNumber;

    std::string masterNameTitle{"Производитель сварочно-монтажных работ с результатами контроля ознакомлен и заключение получил"};
    std::string masterName;
    std::string masterOrganization;
    std::string masterCertNumber;

    static inline const std::string technologicalControlMapTitle{"Контроль выполнен в соответствии с операционной технологической картой"};
    int technologicalControlMapIndex = 0;
    std::vector<std::string> technologicalControlMap = {"ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК"};

    static inline const std::string normativeDocsTitle{"Оценка качества по"};
    std::string normativeDocs{"РД-25.160.10-КТН-0016-23 с Изм.1"};

    static inline const std::string equipmentTitle{"Оборудование и материалы в соответствии с операционной технологической картой"};
    std::string equipment{};

    static inline const std::string weldTypeTitle{"Тип сварного соединения, способ сварки"};
    int weldTypeIndex = 0;
    static inline const std::vector<std::string> weldType{"Стыковое", "Стыковое кольцевое", "Угловое"};
    int weldingMethodIndex = 0;
    std::vector<std::string> weldingMethod{"ручная дуговая", "автоматическая"};

    static inline const std::string diameterTitle{"Диаметр, толщина стенки свариваемых элементов, мм"};
    int diameter = 1220;
    int perimeter = 0;
    float thicknes1 = 12.f;
    float thicknes2 = 14.f;

    static inline const std::string weldersMarkTitle{"Шифр клейма сварщика/бригады сварщиков"};
    std::string weldersMark1{"0763"};
    std::string weldersMark2{""};

    static inline const std::string sectionTypeTitle{"Тип секций (одношовная или двухшовная). Координаты продольных швов, наименьшее расстояние между продольными швами, мм"};
    static inline const std::array<std::string, 3> sectionType1{"Одношовная", "Двухшовная", "Бесшовная"};
    static inline const std::array<std::string, 3> sectionType2{"Одношовная", "Двухшовная", "Бесшовная"};

    static inline const std::string filmNumberTitle{"Номер снимка, координаты мерного пояса, мм"};
    static inline const std::string sensitivityTitle{"Чувствительность"};
    float sensitivity = 0.0f;

    float weldOptDenMin = 1.5f;
    int negatoscopeBrightness = 100000;
    float metalOptDenMax;
    std::string opticalDensityTitle{};

    static inline const std::string opticalDeltaTitle{"Разница оптических плотностей между эталоном чувствительности и основным металлом, е.о.п."};
    float opticalDelta = 0.0f;

    static inline const std::string defectsTitle{"Описание выявленных дефектов"};

    static inline const std::string acceptableTitle{"ЗАКЛЮЧЕНИЕ о допустимости выявленных дефектов (допустим/не допустим)"};
    int acceptableIndex = 0;
    static inline const std::vector<std::string> acceptable{"допустим", "не допустим", "-"};

    static inline const std::string notesTitle{"Примечания"};

    static inline const std::string extentOfUnacceptableDefectsTitle{"Суммарная протяжённость недопустимых дефектов, подлежащих устранению с применением сварки, по всей длине/периметру сварного шва, мм"};
    float extentOfUnacceptableDefects = 0.f;

    static inline const std::string controlResultTitle{"Заключение о годности сварного соединения: («годен», «ремонт», «вырезать», «повторный контроль»)"};
    int controlResultIndex = 0;
    static inline const std::array<std::string, 4> controlResult{"годен", "ремонт", "вырезать", "повторный контроль"};

    std::string sectionNumber1;
    std::string sectionNumber2;
    int coordSec1Weld1 = 0;
    int coordSec1Weld2 = 0;
    int coordSec2Weld1 = 0;
    int coordSec2Weld2 = 0;

    int brightness = 0;
    int temperature = 0;

    static inline const std::array<std::string, 4> roughness{"Rz20", "Rz40", "Rz60", "Rz80"};
    int roughnessIndex = 0;

    float maxHeightOfWeld = 0.f;
    float minHeightOfWeld = 0.f;
    float maxWidthOfWeld = 0.f;
    float minWidthOfWeld = 0.f;
    float edgeDisplacement = 0.f;

    std::vector<DefRT> defRGCList; ///< Список дефектов
};