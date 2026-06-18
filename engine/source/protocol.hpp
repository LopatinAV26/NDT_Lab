#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <optional>
#include "imgui.h"
#include "imgui_stdlib.h"

struct ProtocolData;

class Protocol
{
public:
    virtual ~Protocol() = default;

    virtual void WindowProtocol(bool &showProtocol) = 0;
    virtual void SaveProtocol() = 0;

protected:
private:
};

struct ProtocolData
{
    std::string nameLab{"ЛНК"};
    std::string numberAttestation{"Номер свидетельства об аттестации"};
    std::string weldNumber{"Номер сварного соединения"};
    std::string protocolNumber{"Номер заключения"};
    std::string controlDate{"Дата проведения контроля"};
    std::string dateOfIssue{"Дата выдачи заключения"};

    std::array<std::string_view, 8> methods = {"ВИК", "ПВК", "УК", "РК", "ЦРК", "МК", "ПВТ", "Расслоение"};
    int methodIndex{0};

    std::string objectName{"Объект контроля"};

    std::array<std::string_view, 6> pipeCategory = {"В", "I", "II", "III", "IV", "-"};
    int pipeCategoryIndex{0};

    std::string contractorOrganization{"Подрядная организация"};
    std::string customerOrganization{"Организация заказчика"};

    std::vector<std::string> technologicalControlMap = {"ТК-ТНДВ-ВИК", "ТК-ТНДВ-ПВК", "ТК-ТНДВ-УК"};
    int technologicalControlMapIndex{0};

    std::string equipment{"Средства контроля"};

    std::string normativeDoc{"РД-25.160.10-КТН-0016-23 с Изм.1"};
    std::string weldType{"Тип сварного соединения"};
    std::string weldingMethod{"Способ сварки"};
    int diameter{0};
    float thicknes_1{0.f};
    float thicknes_2{0.f};
    float perimeter{0.f};
    std::array<std::string_view, 3> typeOfSection_1 {"Одношовная", "Двухшовная", "Бесшовная"};
    std::string typeOfSection_2{"Тип секции 2"};
    std::string numberOfSection_1{"-"};
    std::string numberOfSection_2{"-"};
    int coordSec_1Weld_1{0};
    int coordSec_1Weld_2{0};
    int coordSec_2Weld_1{0};
    int coordSec_2Weld_2{0};
    std::string weldersMark_1{"Клеймо сварщика 1"};
    std::string weldersMark_2{"Клеймо сварщика 2"};
    int brightness{0};
    int temperature{0};

    std::vector<std::string> roughness{"Rz20", "Rz40", "Rz60"};
    int roughnessIndex{0};

    // std::string stringOfDefects{""};
    std::vector<std::string> acceptable{"допустим", "не допустим", "-"};
    int acceptableIndex{0};

    float extentOfUnacceptableDefects{0.f}; // Суммарная протяжённость недопуст. дефектов
    float maxHeightOfWeld{0.f};
    float minHeightOfWeld{0.f};
    float maxWidthOfWeld{0.f};
    float minWidthOfWeld{0.f};
    float edgeDisplacement{0.f};

    std::string controllerName{"Контроль провёл"};
    std::string conclusionCreateName{"Заключение выдал"};
    std::string inspectorName{"Надзор"};
    std::string masterName{"Производитель работ"};

    std::vector<std::string> resultOfControl{"годен", "ремонт", "вырезать", "повторный контроль"};
};