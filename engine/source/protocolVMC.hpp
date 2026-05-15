#pragma once

#include "protocolBase.hpp"

class ProtocolVMC : public ProtocolBase
{
public:
    virtual ~ProtocolVMC() = default;

    virtual void WindowProtocol(bool &showProtocol) override;
    virtual void SaveProtocol() override;
    void CreateTable();

private:
    std::shared_ptr<ProtocolData> protocolData = std::make_shared<ProtocolData>();
    // toml::table protocolVMC;

    ImGuiTableColumnFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable;
};