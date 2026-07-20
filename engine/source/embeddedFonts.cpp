#include "embeddedFonts.hpp"
#include "ShareTechMonoRegular.cpp"
/* #include "NotoSansRegular.cpp"
#include "NotoSansBold.cpp"
#include "NotoSansItalic.cpp"
#include "NotoSansBoldItalic.cpp" */

ImFont *LoadEmbeddedShareTechMono(ImGuiIO &io, float sizePx)
{
	// Загружаем и настраиваем шрифт
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 2;	   // Включаем горизонтальный оверсэмплинг для лучшего рендеринга
	fontConfig.OversampleV = 2;	   // Включаем вертикальный оверсэмплинг
	fontConfig.PixelSnapH = false; // Отключаем привязку к пикселям для лучшего дробного масштабирования

	return io.Fonts->AddFontFromMemoryCompressedTTF(
		ShareTechMonoRegular_compressed_data,
		ShareTechMonoRegular_compressed_size,
		sizePx,
		&fontConfig,
		nullptr);
}
