#pragma once

#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct Equipment;

class EquipmentEditWindow
{
public:
    void Show(Equipment &equipment, bool &isOpen);

private:
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
};
