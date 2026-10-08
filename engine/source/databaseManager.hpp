#pragma once

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

struct sqlite3;
struct Employee;
struct Inspector;
struct Master;
struct Welder;
struct Equipment;
struct ControlMap;
struct NormativeDocument;
struct FilmMeasurement;
struct DefectRt;
class Report;
class Laboratory;

namespace NDT
{
    struct DbRecord;
    struct AttachedFile;
}

class DatabaseManager
{
public:
    explicit DatabaseManager(const std::filesystem::path &pathToDb);
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
    DatabaseManager(DatabaseManager &&) = delete;
    DatabaseManager &operator=(DatabaseManager &&) = delete;
    ~DatabaseManager();

    void SaveLaboratoryInfo(const Laboratory &lab);
    void LoadLaboratoryInfo(Laboratory &lab);

    void SaveEmployees(const std::vector<Employee> &employees);
    std::vector<Employee> LoadEmployees();

    void SaveInspectors(const std::vector<Inspector> &inspectors);
    std::vector<Inspector> LoadInspectors();

    void SaveMasters(const std::vector<Master> &masters);
    std::vector<Master> LoadMasters();

    void SaveWelders(const std::vector<Welder> &welders);
    std::vector<Welder> LoadWelders();

    void SaveEquipment(const std::vector<Equipment> &equipmentList);
    std::vector<Equipment> LoadEquipment();

    void SaveControlMaps(const std::vector<ControlMap> &controlMaps);
    std::vector<ControlMap> LoadControlMaps();

    void SaveNormativeDocuments(const std::vector<NormativeDocument> &normativeDocuments);
    std::vector<NormativeDocument> LoadNormativeDocuments();

    /// @brief Прочитать замеры одного заключения
    /// @return порядок замеров восстанавливается сортировкой по id: UUID v7 монотонен по времени создания
    std::vector<FilmMeasurement> LoadFilmMeasurements(const std::string &reportId);

    /// @brief Прочитать дефекты одного заключения в порядке ввода (сортировка по id - UUID v7)
    std::vector<DefectRt> LoadDefectsRt(const std::string &reportId);

    /// @brief Записать заключения вместе с их замерами и дефектами - каждое целиком, в одной транзакции.
    /// Замеры и дефекты не имеют собственной отметки изменения: любая их правка помечает изменённым
    /// заключение, и его список записывается заново. Так же заключение будет уходить и на сервер.
    /// Удалённое заключение остаётся в базе с deleted_at, как и записи справочников
    void SaveReports(const std::vector<Report> &reports);

    /// @brief Прочитать заключения вместе с их замерами и дефектами, в порядке создания
    std::vector<Report> LoadReports();

    /// @brief Запомнить момент, с которого считаются изменения: Save* пишут только записи
    /// с updatedAt не раньше него. Вызывается перед загрузкой и перед каждым сохранением -
    /// время берётся до работы с базой, чтобы правка в ту же секунду не потерялась
    void SetSyncedAt(std::chrono::sys_seconds time) { syncedAt = time; }

    /// @brief Запись менялась после прошлой загрузки или сохранения.
    /// Сравнение нестрогое: updatedAt хранится с точностью до секунды, и правка в ту же секунду,
    /// что и сохранение, при строгом сравнении потерялась бы - лишняя перезапись безвредна
    bool IsChanged(const NDT::DbRecord &record) const;

private:
    /// @brief Сколько записей списка изменилось - при нуле Save* не трогает базу вовсе.
    /// Определение в databaseManager.cpp: шаблон нужен только там
    template <typename T>
    int CountChanged(const std::vector<T> &records) const;

    /// по умолчанию - начало эпохи: пока момент не задан, сохраняется всё
    std::chrono::sys_seconds syncedAt{};

    /// @brief Записать замеры одного заключения так, чтобы в базе остался ровно этот список:
    /// замеры, которых в нём нет, удаляются физически. Своей транзакции не открывает -
    /// вызывается только из SaveReports внутри транзакции заключения.
    /// updated_at/deleted_at замера ни на что не влияют: изменённым считается заключение целиком
    /// @param reportId id заключения - проставляется всем записям, поле FilmMeasurement::reportId не используется
    void SaveFilmMeasurements(const std::string &reportId, const std::vector<FilmMeasurement> &measurements);

    /// @brief Аналогично SaveFilmMeasurements, но для дефектов РК
    /// @param reportId id заключения - проставляется всем записям, поле DefectRt::reportId не используется
    void SaveDefectsRt(const std::string &reportId, const std::vector<DefectRt> &defects);

    /// @brief Создать таблицу laboratory_info, если её ещё нет, и дозаполнить отсутствующие
    /// колонки в уже существующей таблице (ALTER TABLE ADD COLUMN). Таблица хранит одну запись -
    /// SaveLaboratoryInfo/LoadLaboratoryInfo работают с ней через тот же upsert-по-id, что и
    /// Employee/Inspector, поэтому id лаборатории должен сохраняться стабильным между запусками
    void EnsureLaboratoryInfoTable();

    /// @brief Создать таблицу employees, если её ещё нет, и дозаполнить отсутствующие колонки
    /// в уже существующей таблице (ALTER TABLE ADD COLUMN) - на случай, если Employee получил
    /// новое поле после того, как база уже была создана на диске старой версией приложения
    void EnsureEmployeesTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы inspectors
    void EnsureInspectorsTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы masters
    void EnsureMastersTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы welders
    void EnsureWeldersTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы equipment
    void EnsureEquipmentTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы control_maps
    void EnsureControlMapsTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы normative_documents
    void EnsureNormativeDocumentsTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы film_measurements.
    /// Дополнительно создаёт индекс по report_id - выборка всегда идёт по заключению
    void EnsureFilmMeasurementsTable();

    /// @brief Аналогично EnsureFilmMeasurementsTable, но для таблицы defects_rt
    void EnsureDefectsRtTable();

    /// @brief Аналогично EnsureEmployeesTable, но для таблицы reports
    void EnsureReportsTable();

    /// @brief Создать таблицу files - прикреплённые файлы. Строка после вставки не меняется:
    /// другой файл получает новый id (см. NDT::AttachedFile)
    void EnsureFilesTable();

    /// @brief Перенести файлы из колонок file_name/file_data таблицы table в таблицу files
    /// и удалить старые колонки. Для базы, созданной до появления files; повторно ничего не делает
    /// @return true - перенос был, файл базы стоит сжать (VACUUM)
    bool MigrateEmbeddedFiles(const std::string &table);

    /// @brief Записать файл в files, если его там ещё нет. Пустой файл пропускается
    void SaveAttachedFile(const NDT::AttachedFile &file);

    /// @brief Прочитать файл по id. Пустой id или отсутствующая строка - file остаётся пустым
    void LoadAttachedFile(const std::string &fileId, NDT::AttachedFile &file);

    /// @brief Удалить из files файлы, на которые не ссылается ни одна запись (открепили или заменили)
    void DeleteOrphanFiles();

    sqlite3 *db = nullptr;
};