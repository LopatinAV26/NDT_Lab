#include "embeddedFonts.hpp"

#include <array>

#include "ShareTechMonoRegular.cpp"
#include "NotoSansRegular.cpp"
#include "NotoSansBold.cpp"
#include "NotoSansItalic.cpp"
#include "NotoSansBoldItalic.cpp"

std::array<ImFont *, 5> LoadEmbeddedImGuiFonts(ImGuiIO &io, float sizePx)
{
	ImFontConfig fontConfig;
	// OversampleH/V игнорируются растеризатором FreeType (IMGUI_ENABLE_FREETYPE) - там качество
	// обеспечивает хинтинг; значения оставлены на случай сборки со встроенным stb_truetype
	fontConfig.OversampleH = 2;
	fontConfig.OversampleV = 2;
	fontConfig.PixelSnapH = false;
	fontConfig.FontDataOwnedByAtlas = false; // данные встроены статически, ImGui не должен их освобождать
	//fontConfig.RasterizerMultiply = 1.0f; // Устанавливаем множитель для растеризации (по умолчанию 1.0f)

	// Диапазоны глифов (GetGlyphRangesCyrillic) не задаются: начиная с 1.92 атлас динамический
	// и глифы растеризуются по мере надобности - бэкенд SDL_Renderer3 выставляет
	// ImGuiBackendFlags_RendererHasTextures. Так доступна вся кириллица, а не 400 символов из набора

	std::array<ImFont *, 5> fonts;

	fonts.at(0) = io.Fonts->AddFontFromMemoryTTF(NotoSansRegular_data, NotoSansRegular_data_len, sizePx, &fontConfig);
	fonts.at(1) = io.Fonts->AddFontFromMemoryTTF(NotoSansBold_data, NotoSansBold_data_len, sizePx, &fontConfig);
	fonts.at(2) = io.Fonts->AddFontFromMemoryTTF(NotoSansItalic_data, NotoSansItalic_data_len, sizePx, &fontConfig);
	fonts.at(3) = io.Fonts->AddFontFromMemoryTTF(NotoSansBoldItalic_data, NotoSansBoldItalic_data_len, sizePx, &fontConfig);
	fonts.at(4) = io.Fonts->AddFontFromMemoryCompressedTTF(ShareTechMonoRegular_compressed_data, ShareTechMonoRegular_compressed_size, sizePx, &fontConfig);

	return fonts;
}

std::array<PoDoFo::PdfFont *, 4> LoadEmbeddedPDFFonts(PoDoFo::PdfMemDocument &doc)
{
	std::array<PoDoFo::PdfFont *, 4> fonts;

	fonts.at(0) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansRegular_data), NotoSansRegular_data_len));
	fonts.at(1) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBold_data), NotoSansBold_data_len));
	fonts.at(2) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansItalic_data), NotoSansItalic_data_len));
	fonts.at(3) = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBoldItalic_data), NotoSansBoldItalic_data_len));

	return fonts;
}
