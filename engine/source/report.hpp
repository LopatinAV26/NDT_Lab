#pragma once

#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <optional>

#include "utilities.hpp"
#include "methodsNdt.hpp"

enum class DefectRtSymbol : uint8_t
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
    Mw,     /// металлическое включение

    Count /// служебный маркер количества элементов enum
};

struct DefectRt : NDT::DbRecord
{
    std::string reportId; ///< внешний ключ на Report::id - какому заключению принадлежит дефект

    DefectRtSymbol symbol = DefectRtSymbol::Aa;
    float length = 0.f;          ///< протяжённость; для скоплений и цепочек - всей группы
    float width = 0.f;           ///< длина; для скоплений и цепочек - размер a наибольшего включения
    float height = 0.f;          ///< ширина; для скоплений и цепочек - размер b наибольшего включения
    bool endGreaterThan = false; ///< знак оценки глубины: false = "≤", true = ">"
    int coord = 0;               ///< координата начала дефекта по мерному поясу, мм - по ней же участок
    int exposure = 0;            ///< номер экспозиции с нуля - при схеме "на эллипс" вместо координаты
    bool acceptable = true;      ///< недопустимые не склеиваются при печати и пишутся с координатой
};

/// @brief Какие поля входят в запись дефекта данного типа (РД табл. 8.7)
struct DefectRtFields
{
    bool length = false; ///< протяжённость
    bool size = false;   ///< пара "длина×ширина"
    bool sign = false;   ///< знак оценки глубины
};

DefectRtFields GetDefectRtFields(DefectRtSymbol symbol);

/// @brief Округление размера дефекта по РД п. 8.4.20.4: до 3 мм включительно - вверх с шагом 0,1 мм,
/// свыше - вверх с шагом 0,5 мм
float RoundDefectSize(float value);

/// @brief Замеры на одном участке снимка - строка таблицы оптических параметров.
/// Замеров у заключения несколько, поэтому в БД это отдельная таблица со связью
/// по reportId - как у DefectRt
struct FilmMeasurement : NDT::DbRecord
{
    std::string reportId; ///< внешний ключ на Report::id - какому заключению принадлежит замер

    int coord = 0;           ///< координата мерного пояса, мм
    float sensitivity = 0.f; ///< чувствительность контроля, мм
    float weldDensity = 1.5f; ///< оптическая плотность сварного шва, е.о.п.
    float hazDensity = 3.0f;  ///< оптическая плотность околошовной зоны, е.о.п.
    float densityDiff = 0.3f;  ///< разница плотностей между эталоном чувствительности и основным металлом, е.о.п. - измеряется отдельно
};

struct DefUC
{
    int defNameAmplitudeUCIndex = 0;
    int defNameUCIndex = 0;
    static inline const std::array<std::string, 2> defNameAmplitudeUC{"Ад", "Ан"};
    static inline const std::array<std::string, 5> defNameUC{"SH", "LS", "LB", "TD", "CC"};
};

class Report : public NDT::DbRecord
{
public:
    Report();

    /// @brief Получить заголовок отчёта
    /// @param name
    /// @return
    std::string GetMethodReportTitle(Method value) const;
    /// static: данные заключения не нужны, а DatabaseManager вызывает без объекта Report
    static std::string GetDefectRTName(DefectRtSymbol value);

    /// @brief Обратное преобразование к GetDefectRTName - для разбора значения при загрузке из БД
    /// (хранить нужно именно строковый код, а не число enum - см. GetDefectRTName)
    static DefectRtSymbol ParseDefectRtSymbol(const std::string &name);

    /// @brief Наименьшее расстояние между продольными швами двух свариваемых секций, мм.
    /// @brief Швы лежат на окружности стыка, поэтому расстояние меряется по короткой дуге,
    /// @brief а из всех пар швов берётся минимальная
    /// @return nullopt, если хотя бы у одной секции продольных швов нет (бесшовная, фланец)
    std::optional<int> GetMinSeamDistance() const;

    /// @brief Запись дефекта по РД табл. 8.7
    /// @param count сколько одинаковых дефектов описывает запись - печатается перед обозначением, если больше 1
    std::string GetDefectRecord(const DefectRt &defect, int count = 1) const;

    static constexpr int sectionLength = 300; ///< участок мерного пояса, мм - строка таблицы дефектов заключения

    /// @brief Количество участков по sectionLength на длине шва; последний может быть короче
    int GetLengthSectionCount() const;

    /// @brief Строки таблицы - снимки, а не участки по 300 мм: на коротком шве участков не больше,
    /// чем замеров, и несколько замеров попали бы в одну строку
    bool IsSplitByFilms() const;

    /// @brief Количество строк таблицы дефектов: участки по sectionLength, на коротком шве - снимки
    /// (по одному на четверть, см. IsSplitByFilms), а при схеме "на эллипс" - экспозиции
    int GetSectionCount() const;

    /// @brief Чем является строка таблицы дефектов: "Участок", "Снимок" или "Экспозиция" - для заголовков в окнах
    std::string GetSectionTitle() const;

    /// @brief Координата начала строки таблицы по мерному поясу, мм
    int GetSectionStart(int section) const;

    /// @brief Номер строки таблицы, в которую попадает координата мерного пояса
    int GetSection(int coord) const;

    /// @brief Строка таблицы, к которой относится дефект: по координате, а на эллипс - по экспозиции
    int GetDefectSection(const DefectRt &defect) const;

    /// @brief Подпись строки для заключения: "0-300", ..., последний участок замыкается на ноль - "3600-0";
    /// на коротком шве - номер снимка и его участок "1 (0-125)"...; на эллипс - "1 экспозиция", "2 экспозиция"...
    std::string GetSectionRangeStr(int section) const;

    /// @brief Сколько замеров оптических параметров требует схема; на эллипс - по одному на экспозицию
    int GetMeasurementCount() const;

    /// @brief Расставить координаты замеров по центрам равных долей шва: при четырёх замерах -
    /// по центрам четвертей (для длины 500 мм - 62, 187, 312, 437)
    void SetDefaultMeasurementCoords();

    /// @brief Описание дефектов участка: одинаковые допустимые склеены с количеством, записи через "; ", без дефектов - "-"
    std::string GetSectionDefectsStr(int section) const;

    /// @brief Участок допустим, если на нём не начинается ни один недопустимый дефект
    bool IsSectionAcceptable(int section) const;

    /// @brief Замер оптических параметров, сделанный на этом участке
    /// @return nullptr, если на участке замера нет - колонки 7-9 в этой строке пустые
    const FilmMeasurement *GetSectionMeasurement(int section) const;

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

    std::string pipeCategoryTitle{"Категория трубопровода"};
    Category pipeCategory;

    std::string contractorOrganizationTitle{"Подрядная организация"};
    std::string contractorOrganization;

    std::string customerOrganizationTitle{"Организация заказчика"};
    std::string customerOrganization;

    Method methodValue;
    std::string methodHeader; ///< заголовок формируется при выборе метода

    std::string controllerNameTitle{"Контроль произвёл"};
    std::string controllerName; ///< дефектоскопист, который произвёл контроль
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
    std::string technologicalControlMap{}; ///< шифр выбранной техкарты (ControlMap::code)

    static inline const std::string normativeDocsTitle{"Оценка качества по"};
    std::string normativeDocs{};                 ///< шифры выбранных нормативных документов через запятую
    std::vector<std::string> normativeDocsIds{}; ///< id выбранных документов - из них строится normativeDocs и восстанавливаются галочки

    static inline const std::string equipmentTitle{"Оборудование и материалы в соответствии с операционной технологической картой"};
    std::string equipment{};                 ///< готовая строка для заключения: "Наименование №зав.номер" через запятую
    std::vector<std::string> equipmentIds{}; ///< id выбранного оборудования - из них строится equipment и восстанавливаются галочки

    static inline const std::string weldTypeTitle{"Тип сварного соединения, способ сварки"};
    WeldJointType weldType; ///< обозначение для бланка - GetWeldJointTypeStr
    std::vector<WeldingMethod> weldingMethods{}; ///< способов может быть несколько: корень и заполнение варят по-разному

    static inline const std::string diameterTitle{"Диаметр, толщина стенки свариваемых элементов, мм"};
    int diameter = 1220;
    int perimeter = 0;
    float thicknes1 = 12.f;
    float thicknes2 = 14.f;

    static inline const std::string weldersMarkTitle{"Шифр клейма сварщика/бригады сварщиков"};
    std::string weldersMark{};             ///< шифры клейм для бланка, каждый с новой строки
    std::vector<std::string> weldersIds{}; ///< id выбранных сварщиков - из них строится weldersMark и восстанавливаются галочки

    static inline const std::string sectionTypeTitle{"Тип секций (одношовная или двухшовная). Координаты продольных швов, наименьшее расстояние между продольными швами, мм"};
    SectionType sectionType1; ///< тип секции слева от стыка, обозначение для бланка - GetSectionTypeStr
    SectionType sectionType2; ///< тип секции справа от стыка

    std::string sectionNumber1;
    std::string sectionNumber2;
    int coordSec1Weld1 = 0;
    int coordSec1Weld2 = 0;
    int coordSec2Weld1 = 0;
    int coordSec2Weld2 = 0;

    static inline const std::string filmNumberTitle{"Номер снимка, координаты мерного пояса, мм"};

    static inline const std::string sensitivityTitle{"Чувствительность"};

    ExposureScheme exposureScheme;

    static constexpr int maxEllipseExposures = 4;
    int ellipseExposureCount = 2; ///< экспозиций при схеме "на эллипс": от GetFilmMeasurementCount(Ellipse) до maxEllipseExposures

    /// количество строк задаёт схема просвечивания, отсюда вектор, а не массив
    std::vector<FilmMeasurement> filmMeasurements{};

    float weldOptDenMin = 1.5f; ///< минимальная оптическая плотность самого светлого участка шва, е.о.п.
    int negatoscopeBrightness = 100000;
    float metalOptDenMax;
    std::string opticalDensityTitle;

    static inline const std::string opticalDiffTitle{"Разница оптических плотностей между эталоном чувствительности и основным металлом, е.о.п."};
    
    static inline const std::string defectsTitle{"Описание выявленных дефектов"};

    static inline const std::string acceptableTitle{"ЗАКЛЮЧЕНИЕ о допустимости выявленных дефектов (допустим/не допустим)"};

    static inline const std::string notesTitle{"Примечания"};

    static inline const std::string extentOfUnacceptableDefectsTitle{"Суммарная протяжённость недопустимых дефектов, подлежащих устранению с применением сварки, по всей длине/периметру сварного шва, мм"};
    float extentOfUnacceptableDefects = 0.f;

    static inline const std::string controlResultTitle{"Заключение о годности сварного соединения: («годен», «ремонт», «вырезать», «повторный контроль»)"};
    ControlResult controlResult = ControlResult::Fit;

    


    // Для ВИК/////////////////////
    int brightness = 0;
    int temperature = 0;

    Roughness roughness = Roughness::Rz20;

    float maxHeightOfWeld = 0.f;
    float minHeightOfWeld = 0.f;
    float maxWidthOfWeld = 0.f;
    float minWidthOfWeld = 0.f;
    float edgeDisplacement = 0.f;
    //////////////////////////////////

    std::vector<DefectRt> defRGCList; ///< Список дефектов
    std::vector<std::string> sectionNotes{}; ///< примечания по участкам, индекс - номер участка
};