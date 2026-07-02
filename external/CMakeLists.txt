include(FetchContent)

# SDL STATIC BUILD
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# SDL3 ---------------------------------------------------------------------------------------
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.10
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# SDL_image ---------------------------------------------------------------------------------------
#set(SDLIMAGE_INSTALL OFF CACHE BOOL "" FORCE)
#set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
#set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)
#set(SDLIMAGE_VENDORED ON CACHE BOOL "" FORCE)

# TIFF отключаем: движок его не грузит через SDL_image, а вендоренный libtiff
# создаёт таргет TIFF::TIFF, который конфликтует с TIFF::TIFF из vcpkg (нужен podofo).
#set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)

# Реальный libpng тоже отключаем по той же причине: SDL_image создаёт свой
# PNG::PNG (алиас на вендоренный shared png), который тихо (без ошибки конфигурации)
# подменяет собой PNG::PNG из vcpkg, нужный podofo/freetype - в итоге в бинарник
# просачивается динамическая libpng16.so. PNG всё равно грузится через встроенный stb.
#set(SDLIMAGE_PNG_LIBPNG OFF CACHE BOOL "" FORCE)

#FetchContent_Declare(SDL3_image
#    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
#    GIT_TAG release-3.4.4
#    GIT_SHALLOW TRUE
#)
#FetchContent_MakeAvailable(SDL3_image)

# toml++ ------------------------------------------------------------------------------------
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(tomlplusplus)

# imgui --------------------------------------------------------------------------------------
add_library(imgui STATIC)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_stdlib.cpp
    imgui/imgui_impl_sdl3.cpp
    imgui/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

target_link_libraries(imgui PUBLIC SDL3::SDL3-static)

target_compile_options(imgui PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

# implot -------------------------------------------------------------------------------------
add_library(implot STATIC)

target_sources(implot PRIVATE
    implot/implot.cpp
    implot/implot_items.cpp
)

target_include_directories(implot SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/implot")

target_link_libraries(implot PUBLIC imgui)

target_compile_options(implot PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

# sqlite -------------------------------------------------------------------------------------
add_library(sqlite3 STATIC sqlite/sqlite3.c)

target_include_directories(sqlite3 SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sqlite")

target_compile_options(sqlite3 PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-w>
)

# libpqxx (vcpkg) ----------------------------------------------------------------------------
# GLOBAL нужен, чтобы IMPORTED-таргет был виден в engine/ (соседняя, не дочерняя директория)
find_package(libpqxx CONFIG REQUIRED GLOBAL)

# podofo (vcpkg) -------------------------------------------------------------------------------
find_package(podofo CONFIG REQUIRED GLOBAL)