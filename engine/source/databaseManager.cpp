#include "databaseManager.hpp"
#include <SDL3/SDL.h>
#include "sqlite3.h"

DatabaseManager::DatabaseManager(const std::filesystem::path &pathToDb)
{
    if (sqlite3_open_v2(pathToDb.string().c_str(), &db, SQLITE_OPEN_READWRITE, nullptr) != SQLITE_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open database '%s': %s",
                     pathToDb.string().c_str(), sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return;
    }

    SDL_Log("Database '%s' opened successfully.", pathToDb.string().c_str());
}

DatabaseManager::~DatabaseManager()
{
    sqlite3_close(db);
}