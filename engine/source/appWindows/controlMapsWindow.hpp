#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct ControlMap;

class ControlMapsWindow
{
public:
    void Show(std::vector<ControlMap> &controlMapsList);

private:
    void Edit(ControlMap &controlMap, bool &isOpen);
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
    bool editWindow = false;
};
