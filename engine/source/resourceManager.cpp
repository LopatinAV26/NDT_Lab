#include "resourceManager.hpp"

void LoadDevices(const std::string_view &pathToDevices)
{
    toml::table tbl = toml::parse_file(pathToDevices);
    auto devicesArray = tbl["devices"].as_array();
}
