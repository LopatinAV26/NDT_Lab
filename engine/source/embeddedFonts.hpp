#pragma once

#include "imgui.h"
#include <podofo/podofo.h>
#include <array>

std::array<ImFont*, 5> LoadEmbeddedImGuiFonts(ImGuiIO& io, float sizePx);
std::array<PoDoFo::PdfFont*, 4> LoadEmbeddedPDFFonts(PoDoFo::PdfMemDocument& doc);