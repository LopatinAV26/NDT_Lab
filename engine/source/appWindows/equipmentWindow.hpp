#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct Equipment;

class EquipmentWindow
{
public:
    void Show(std::vector<Equipment> &equipmentList);

private:
    void Edit(Equipment &equipment, bool &isOpen);
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
    bool editWindow = false;
};
