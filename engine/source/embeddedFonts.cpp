#include "embeddedFonts.hpp"
#include "ShareTechMonoRegular.cpp"
#include "NotoSansRegular.cpp"
#include "NotoSansBold.cpp"
#include "NotoSansItalic.cpp"
#include "NotoSansBoldItalic.cpp"

std::array<ImFont *, 5> LoadEmbeddedImGuiFonts(ImGuiIO &io, float sizePx)
{
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;
	fontConfig.OversampleV = 2;
	fontConfig.PixelSnapH = false;
	fontConfig.FontDataOwnedByAtlas = false; // данные встроены статически, ImGui не должен их освобождать
	//fontConfig.RasterizerMultiply = 1.0f; // Устанавливаем множитель для растеризации (по умолчанию 1.0f)

	const ImWchar *glyphRanges = io.Fonts->GetGlyphRangesCyrillic();

	std::array<ImFont *, 5> fonts;

	fonts.at(0) = io.Fonts->AddFontFromMemoryTTF(NotoSansRegular_data, NotoSansRegular_data_len, sizePx, &fontConfig, glyphRanges);
	fonts.at(1) = io.Fonts->AddFontFromMemoryTTF(NotoSansBold_data, NotoSansBold_data_len, sizePx, &fontConfig, glyphRanges);
	fonts.at(2) = io.Fonts->AddFontFromMemoryTTF(NotoSansItalic_data, NotoSansItalic_data_len, sizePx, &fontConfig, glyphRanges);
	fonts.at(3) = io.Fonts->AddFontFromMemoryTTF(NotoSansBoldItalic_data, NotoSansBoldItalic_data_len, sizePx, &fontConfig, glyphRanges);
	fonts.at(4) = io.Fonts->AddFontFromMemoryCompressedTTF(ShareTechMonoRegular_compressed_data, ShareTechMonoRegular_compressed_size, sizePx, &fontConfig, glyphRanges);

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
