#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct ControlMap;

class Laboratory;

class ControlMapsWindow
{
public:
    void Show(std::vector<ControlMap> &controlMapsList, const Laboratory &lab);

private:
    void Edit(ControlMap &controlMap, bool &isOpen, bool isUnsaved);
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
    bool editWindow = false;
};
