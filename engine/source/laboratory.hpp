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

struct Employee ///< Сотрудники (ЛНК и надзор)
{
	std::string id = NDT::GenerateUuidV4(); // UUID v4, генерируется на клиенте при создании записи
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
	std::string certificateEndDateVT;
	std::string certificateEndDateUT;
	std::string certificateEndDateRT;
	std::string certificateEndDatePT;
	std::string certificateEndDateMT;
	std::string certificateEndDateLT;
};

class Laboratory
{
public:
	explicit Laboratory(ApplicationData &appData);

	void SaveDB();

	std::vector<Employee> employeesList;					/// список сотрудников
	static inline std::vector<Report> reportList; /// Список отчётов

private:
	std::unique_ptr<DatabaseManager> dbManager;
};