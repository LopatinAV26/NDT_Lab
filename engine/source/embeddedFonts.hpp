#pragma once

#include "imgui.h"
#include <podofo/podofo.h>
#include <array>

ImFont* LoadEmbeddedShareTechMono(ImGuiIO& io, float sizePx);
std::array<ImFont*, 4> LoadEmbeddedNotoSansImGuiFonts(ImGuiIO& io, float sizePx);
std::array<PoDoFo::PdfFont*, 4> LoadEmbeddedNotoSansFonts(PoDoFo::PdfMemDocument& doc);