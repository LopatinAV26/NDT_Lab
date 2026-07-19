#include "protocolData.hpp"

uint ProtocolData::GetPerimeter(uint diam)
{
    return diam * M_PI;
}

float ProtocolData::GetMetalDensity(uint negBright)
{
    return std::log10(negBright) - 2.0f;
    opticalDensityTitle = "";
}
