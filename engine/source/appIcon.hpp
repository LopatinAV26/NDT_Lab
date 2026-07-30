#pragma once

struct SDL_Surface;

/// @brief Создаёт SDL_Surface из встроенных в бинарник RGBA-пикселей иконки (64x64).
/// Вызывающий код владеет поверхностью и должен освободить её через SDL_DestroySurface.
SDL_Surface *CreateAppIconSurface();
