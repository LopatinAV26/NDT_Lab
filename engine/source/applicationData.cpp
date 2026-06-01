#include "applicationData.hpp"

ApplicationData::ApplicationData()
{
    LoadSettings();
}

ApplicationData::~ApplicationData()
{
    SaveSettings();
}
