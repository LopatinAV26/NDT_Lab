#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <cstdint>

#include "utilities.hpp"
#include "databaseManager.hpp"
#include "report.hpp"

class ApplicationData;

struct Employee ///< Сотрудник ЛНК
{
	std::string id = NDT::GenerateUuidV7(); // UUID v7, генерируется на клиенте при создании записи
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()); // когда запись последний раз менялась
	std::optional<std::chrono::sys_seconds> deletedAt;								// nullopt = не удалена; иначе - момент "мягкого" удаления
	std::string name;
	std::string organization;
	std::string department;
	std::string position;
	std::string employeementDate;
	std::string personalCode;
	std::string level;
	std::string certificateNumber;
	std::string certificateDate;
	bool hasVT = true;
	std::string certificateEndDateVT;
	bool hasUT = true;
	std::string certificateEndDateUT;
	bool hasRT = true;
	std::string certificateEndDateRT;
	bool hasPT = true;
	std::string certificateEndDatePT;
	bool hasMT = true;
	std::string certificateEndDateMT;
	bool hasLT = true;
	std::string certificateEndDateLT;
};

struct Inspector ///< Сотрудник надзора
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;
	std::string name;
	std::string organization;
	std::string certificateNumber;
	std::string certificateDate;
	bool hasVT = true;
	std::string certificateEndDateVT;
	bool hasUT = true;
	std::string certificateEndDateUT;
	bool hasRT = true;
	std::string certificateEndDateRT;
	bool hasPT = true;
	std::string certificateEndDatePT;
	bool hasMT = true;
	std::string certificateEndDateMT;
	bool hasLT = true;
	std::string certificateEndDateLT;
};

struct Master ///< Производитель СМР
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;
	std::string name;
	std::string organization;
	std::string department;
	std::string position;
	std::string certificateNumber;
	std::string certificateEndDate;
};

struct Welder ///< Сварщик
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;
	std::string name;
	std::string organization;
	std::string department;
	std::string position;
	std::string personalCode; /// Шифр клейма
	std::string certificateNumber;
	std::string certificateEndDate;
};

struct ControlMap /// Технологическая карта контроля
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;

	std::string code; /// шифр техкарты
	int diameter = 0;
	float thickness = 0.f;
	std::string description;
	bool forVT = false;
	bool forUT = false;
	bool forRT = false;
	bool forDRT = false;
	bool forPT = false;
	bool forMT = false;
	bool forLT = false;
	bool forDT = false;
	bool categoryB = false;
	bool categoryI = false;
	bool categoryII = false;
	bool categoryIII = false;
	bool categoryIV = false;

	std::string fileName;				/// исходное имя прикреплённого файла (сохраняет расширение)
	std::vector<std::uint8_t> fileData; /// содержимое файла целиком (PDF/скан/Word)
};

struct NormativeDocument
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;

	std::string code;	/// шифр
	std::string name;	/// название документа
	std::string method; /// метод контроля
	std::string status; /// статус (действующий, отменён, заменён)
	int year;	/// год введения
	bool forVT = false;
	bool forUT = false;
	bool forRT = false;
	bool forDRT = false;
	bool forPT = false;
	bool forMT = false;
	bool forLT = false;
	bool forECT = false;

	std::string fileName;
	std::vector<std::uint8_t> fileData;
};

struct Equipment /// оборудование лаборатории по "СДАНК-01-2020"
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;

	std::string name;									 /// Наименование оборудования
	std::string method;									 /// Метод контроля, для которого используется оборудование
	std::string function;								 /// назначение
	std::string manufacturer;							 /// Производитель
	std::string serialNumber;							 /// Серийный номер
	std::string yearOfManufacture;						 /// Год выпуска
	std::string yearOfCommissioning;					 /// Год ввода в эксплуатацию
	std::string technicalAndMetrologicalCharacteristics; /// Технические и метрологические характеристики
	std::string owner;									 /// Владелец
	std::string certificateNumber;						 /// Номер документа о поверке/калибровке
	std::string certificateDate;						 /// Дата документа о поверке/калибровке
	std::string certificateEndDate;						 /// Дата окончания действия документа о поверке/калибровке
	std::string state;									 /// Состояние оборудования: (И - исправны (используют в работе); К - законсеpвиpованы (в pаботе не используют); Р - подлежат ремонту; С - подлежат списанию.)
	bool forVT = false;
	bool forUT = false;
	bool forRT = false;
	bool forDRT = false;
	bool forPT = false;
	bool forMT = false;
	bool forLT = false;
	bool forECT = false;

	bool isOperational = true;		/// исправно
	bool isUnderRepair = false;		/// в ремонте
	bool isFaulty = false;			/// неисправно
	bool isPendingDisposal = false; /// подлежит списанию
	bool isPreserved = false;		/// законсервировано

	bool isCalibrated = false; /// Поверяется/калибруется

	std::string fileName;				/// исходное имя прикреплённого файла (сохраняет расширение)
	std::vector<std::uint8_t> fileData; /// содержимое файла целиком (паспорт/формуляр и т.п.)
};

struct LaboratoryInfo
{
	std::string id = NDT::GenerateUuidV7();
	std::chrono::sys_seconds updatedAt =
		std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
	std::optional<std::chrono::sys_seconds> deletedAt;

	std::string laboratoryName;
	std::string numberAttestation;
	std::string attestationEndDate;
};

class Laboratory
{
public:
	explicit Laboratory(ApplicationData &appData);

	void LoadDB();
	void SaveDB();

	LaboratoryInfo labInfo;
	std::vector<Employee> employeesList;				   /// список сотрудников
	std::vector<Inspector> inspectorsList;				   /// список сотрудников надзора
	std::vector<Master> mastersList;					   /// производители СМР
	std::vector<Welder> weldersList;					   /// сварщики
	std::vector<Report> reportsList;					   /// список отчётов
	std::vector<ControlMap> controlMapsList;			   /// тех. карты
	std::vector<Equipment> equpmentsList;				   /// список оборудования
	std::vector<NormativeDocument> normativeDocumentsList; /// нормативные документы

private:
	std::unique_ptr<DatabaseManager> dbManager;
};