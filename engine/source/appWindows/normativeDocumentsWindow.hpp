#pragma once

#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include "imgui.h"

struct NormativeDocument;

class NormativeDocumentsWindow
{
public:
    void Show(std::vector<NormativeDocument> &normativeDocumentsList);

private:
    void Edit(NormativeDocument &normativeDocument, bool &isOpen);
    static void SDLCALL OnFileSelected(void *userdata, const char *const *filelist, int filter);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse;
    std::string pendingFilePath;
    bool fileDialogResultReady = false;
    bool editWindow = false;
};
