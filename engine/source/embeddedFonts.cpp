#include "embeddedFonts.hpp"
#include "ShareTechMonoRegular.cpp"
#include "NotoSansRegular.cpp"
#include "NotoSansBold.cpp"
#include "NotoSansItalic.cpp"
#include "NotoSansBoldItalic.cpp"

ImFont *LoadEmbeddedShareTechMono(ImGuiIO &io, float sizePx)
{
	// Загружаем и настраиваем шрифт
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;	   // Включаем горизонтальный оверсэмплинг для лучшего рендеринга
	fontConfig.OversampleV = 2;	   // Включаем вертикальный оверсэмплинг
	fontConfig.PixelSnapH = false; // Отключаем привязку к пикселям для лучшего дробного масштабирования
	//fontConfig.RasterizerMultiply = 1.0f; // Устанавливаем множитель для растеризации (по умолчанию 1.0f)

	return io.Fonts->AddFontFromMemoryCompressedTTF(
		ShareTechMonoRegular_compressed_data,
		ShareTechMonoRegular_compressed_size,
		sizePx,
		&fontConfig,
		io.Fonts->GetGlyphRangesCyrillic());
}

std::array<ImFont *, 4> LoadEmbeddedNotoSansImGuiFonts(ImGuiIO &io, float sizePx)
{
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;
	fontConfig.OversampleV = 2;
	fontConfig.PixelSnapH = false;
	fontConfig.FontDataOwnedByAtlas = false; // данные встроены статически, ImGui не должен их освобождать

	const ImWchar *glyphRanges = io.Fonts->GetGlyphRangesCyrillic();

	std::array<ImFont *, 4> fonts;

	fonts[0] = io.Fonts->AddFontFromMemoryTTF(NotoSansRegular_data, NotoSansRegular_data_len, sizePx, &fontConfig, glyphRanges);
	fonts[1] = io.Fonts->AddFontFromMemoryTTF(NotoSansBold_data, NotoSansBold_data_len, sizePx, &fontConfig, glyphRanges);
	fonts[2] = io.Fonts->AddFontFromMemoryTTF(NotoSansItalic_data, NotoSansItalic_data_len, sizePx, &fontConfig, glyphRanges);
	fonts[3] = io.Fonts->AddFontFromMemoryTTF(NotoSansBoldItalic_data, NotoSansBoldItalic_data_len, sizePx, &fontConfig, glyphRanges);

	return fonts;
}

std::array<PoDoFo::PdfFont *, 4> LoadEmbeddedNotoSansFonts(PoDoFo::PdfMemDocument &doc)
{
	std::array<PoDoFo::PdfFont *, 4> fonts;

	fonts[0] = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansRegular_data), NotoSansRegular_data_len));
	fonts[1] = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBold_data), NotoSansBold_data_len));
	fonts[2] = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansItalic_data), NotoSansItalic_data_len));
	fonts[3] = &doc.GetFonts().GetOrCreateFontFromBuffer(
		PoDoFo::bufferview(reinterpret_cast<const char *>(NotoSansBoldItalic_data), NotoSansBoldItalic_data_len));

	return fonts;
}
