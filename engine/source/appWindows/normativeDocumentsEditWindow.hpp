#pragma once

#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct NormativeDocument;

class NormativeDocumentsEditWindow
{
public:
    void Show(NormativeDocument &normativeDocument, bool &isOpen);

private:
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
};
