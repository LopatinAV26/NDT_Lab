#include "applicationData.hpp"

ApplicationData::ApplicationData()
{
    LoadSettings();
}

ApplicationData::~ApplicationData()
{
    SaveSettings();
}

void ApplicationData::LoadSettings()
{
    try
    {
        tbl = toml::parse_file(pathToSettings.string());
        SDL_Log("Config file '%s' parsed successfully.\n", pathToSettings.string().c_str());
    }
    catch (const toml::parse_error &err)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to parse config file '%s': %s\n",
                     pathToSettings.string().c_str(), err.what());
        return;
    }

    mainScale = tbl["mainScale"].value_or(mainScale);
    windowRounding = tbl["windowRounding"].value_or(windowRounding);
    frameRounding = tbl["frameRounding"].value_or(frameRounding);
    grabRounding = tbl["grabRounding"].value_or(grabRounding);
    fontSize = tbl["fontSize"].value_or(fontSize);
    style = tbl["guiStyle"].value_or(style);
}

void ApplicationData::SaveSettings()
{
    tbl.insert_or_assign("mainScale", mainScale);
    tbl.insert_or_assign("windowRounding", windowRounding);
    tbl.insert_or_assign("frameRounding", frameRounding);
    tbl.insert_or_assign("grabRounding", grabRounding);
    tbl.insert_or_assign("fontSize", fontSize);
    tbl.insert_or_assign("guiStyle", style);
    std::ofstream file(pathToSettings);
    if (!file)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to open '%s' for writing",
                     pathToSettings.string().c_str());
        return;
    }
    file << tbl;
    SDL_Log("Settings saved to '%s'", pathToSettings.string().c_str());
}
