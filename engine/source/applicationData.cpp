#include "applicationData.hpp"
#include <toml++/toml.hpp>

ApplicationData::ApplicationData()
{
    LoadSettings();
}

namespace
{
    /// @brief Синхронизировать поле между toml-таблицей и переменной
    /// @param loading true - загрузка, false - сохранение
    template <typename T>
    void Sync(toml::table &tbl, const char *key, T &value, bool loading)
    {
        if (loading)
            value = tbl[key].value_or(value);
        else
            tbl.insert_or_assign(key, value);
    }

    /// @brief Синхронизировать все поля ApplicationData с toml-таблицей
    /// @param loading true - загрузка, false - сохранение
    void SyncAllSettings(toml::table &tbl, ApplicationData &appData, bool loading)
    {
        Sync(tbl, "mainScale", appData.mainScale, loading);
        Sync(tbl, "windowRounding", appData.windowRounding, loading);
        Sync(tbl, "frameRounding", appData.frameRounding, loading);
        Sync(tbl, "grabRounding", appData.grabRounding, loading);
        Sync(tbl, "fontSize", appData.fontSize, loading);
        Sync(tbl, "guiStyle", appData.style, loading);
        Sync(tbl, "vsyncMode", appData.mode, loading);
    }
}

void ApplicationData::LoadSettings(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                    "Config file '%s' not found, using default settings.\n",
                    path.string().c_str());
        return;
    }

    toml::table tbl;
    try
    {
        tbl = toml::parse_file(path.string());
        SDL_Log("Config file '%s' parsed successfully.\n", path.string().c_str());
    }
    catch (const toml::parse_error &err)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to parse config file '%s': %s\n",
                     path.string().c_str(), err.what());
        return;
    }

    SyncAllSettings(tbl, *this, true);
}

void ApplicationData::SaveSettings(const std::filesystem::path &path)
{
    toml::table tbl;
    SyncAllSettings(tbl, *this, false);
    std::ofstream file(path);
    if (!file)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to open '%s' for writing",
                     path.string().c_str());
        return;
    }
    file << tbl;
    SDL_Log("Settings saved to '%s'", path.string().c_str());
}

ApplicationData::~ApplicationData()
{
    SaveSettings();
}