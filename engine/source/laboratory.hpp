#pragma once

#include <memory>
#include <string>
#include <format>
#include <array>
#include <vector>
#include <chrono>

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
	std::string personalCode;
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
	std::string personalCode;
	std::string certificateNumber;
	std::string certificateEndDate;
};

struct ControlMap ///Технологическая карта контроля
{
	std::string name;
	std::string code;
	std::string method;
};

struct Equipment
{
	std::string name;

};

struct LaboratoryInfo{
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

	void SaveDB();

	LaboratoryInfo labInfo;
	std::vector<Employee> employeesList;   /// список сотрудников
	std::vector<Inspector> inspectorsList; /// список сотрудников надзора
	std::vector<Master> mastersList;		   /// производители СМР
	std::vector<Welder> weldersList;		   /// сварщики
	std::vector<Report> reportsList;		   /// список отчётов

private:
	std::unique_ptr<DatabaseManager> dbManager;
};