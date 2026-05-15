#include "protocolVMC.hpp"

void ProtocolVMC::WindowProtocol(bool &showProtocol)
{
    ImGui::Begin("Заключение ВИК", &showProtocol);
    {
        if (ImGui::Button("Загрузить данные по умолчанию"))
        {
            ProtocolBase::GetDefaultProtocolData();        // Эта ф-я вызывается здесь для динамического обновления данных из файла по кнопке
            protocolData = ProtocolBase::baseProtocolData; //!!! не верный подход наверное
        }

        ImVec2 sizeTextField{0.f, 70.f};

        ImGui::InputTextMultiline("Наименование ЛНК", &protocolData->nameLab, sizeTextField, ImGuiInputTextFlags_WordWrap);

        ImGui::InputText("Номер свидетельства об аттестации", &protocolData->numberAttestation);

        ImGui::InputTextMultiline("Наименование объекта", &protocolData->objectName, sizeTextField, ImGuiInputTextFlags_WordWrap);

        if (ImGui::BeginCombo("Категория трубопровода",
                              protocolData->pipeCategoryVector.at(protocolData->pipeCategoryIndex).c_str()))
        {
            for (int i = 0; i < protocolData->pipeCategoryVector.size(); ++i)
            {
                const bool isSelected = (protocolData->pipeCategoryIndex == i);
                if (ImGui::Selectable(protocolData->pipeCategoryVector.at(i).c_str(), isSelected))
                    protocolData->pipeCategoryIndex = i;

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::InputTextMultiline("Подрядная организация", &protocolData->contractorOrganization, sizeTextField, ImGuiInputTextFlags_WordWrap);

        ImGui::InputTextMultiline("Организация заказчика", &protocolData->customerOrganization, sizeTextField, ImGuiInputTextFlags_WordWrap);

        ImGui::InputText("Дата выдачи заключения", &protocolData->dateOfIssue);

        ImGui::InputText("Дата проведения контроля", &protocolData->controlDate);

        ImGui::InputText("Номер сварного соединения", &protocolData->weldNumber);
    }

    ImGui::End();
}

void ProtocolVMC::SaveProtocol()
{
    /* configVMC = toml::table{{"Технологическая карта", data->techCard}};
    std::cout << configVMC << std::endl;
    std::ofstream of("/home/av26/Programming/SDL3_ImGUI/build/Debug/resources/config/protocol_VIC_default.toml");
    of << configVMC; */
}

void ProtocolVMC::CreateTable()
{
    ImGui::BeginTable("tableVMC", 1, tableFlags);
    {

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Наименование ЛНК");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(protocolData->nameLab.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Номер свидетельства об аттестации");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(protocolData->numberAttestation.c_str());
        ImGui::EndTable();
    }

    ImGui::SameLine(0, 0);

    (ImGui::BeginTable("tableVMC", 1, tableFlags));
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Наименование ЛНК");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(protocolData->nameLab.c_str());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Номер свидетельства об аттестации");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(protocolData->numberAttestation.c_str());

        ImGui::EndTable();
    }
}
