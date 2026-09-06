include(FetchContent)

# SDL STATIC BUILD
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# SDL3's own check_symbol_exists(getresuid/getresgid "unistd.h" ...) тест компилируется
# без _GNU_SOURCE и не находит эти функции в glibc (они спрятаны за __USE_GNU), поэтому
# SDL_gtk.c определяет свои static inline фолбэки - но при реальной компиляции этого файла
# _GNU_SOURCE всё-таки включается транзитивно (через GTK/glib-заголовки), и настоящие extern
# из unistd.h конфликтуют со static-фолбэком SDL. Форсируем "найдено", чтобы SDL не подставляла
# фолбэк - тогда останется только настоящая glibc-декларация.
set(HAVE_GETRESUID 1 CACHE INTERNAL "" FORCE)
set(HAVE_GETRESGID 1 CACHE INTERNAL "" FORCE)

# SDL3 ---------------------------------------------------------------------------------------
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.14
    #GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# toml++ ------------------------------------------------------------------------------------
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    #GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(tomlplusplus)

# stb_image ------------------------------------------------------------------------------------
add_library(stb_image STATIC stb_image/stb_image.cpp)

target_include_directories(stb_image SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/stb_image")

target_compile_options(stb_image PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

# imgui --------------------------------------------------------------------------------------
# FreeType из vcpkg (ставится как зависимость podofo) - растеризатор шрифтов вместо stb_truetype:
# настоящий хинтинг и более чистая кириллица на мелких размерах
find_package(Freetype REQUIRED)

add_library(imgui STATIC)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_stdlib.cpp
    # путь misc/freetype/ сохранён как в апстриме: imgui_draw.cpp инклюдит
    # "misc/freetype/imgui_freetype.h" под IMGUI_ENABLE_FREETYPE
    imgui/misc/freetype/imgui_freetype.cpp
    imgui/imgui_impl_sdl3.cpp
    imgui/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# PUBLIC, а не через правку imconfig.h: определение переживает обновление ImGui,
# вендорные файлы остаются нетронутыми
target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE)

target_link_libraries(imgui PUBLIC SDL3::SDL3-static Freetype::Freetype)

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

# imGuiDatePicker -----------------------------------------------------------------------------
add_library(imGuiDatePicker STATIC imGuiDatePicker/ImGuiDatePicker.cpp)

target_include_directories(imGuiDatePicker SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imGuiDatePicker")

target_link_libraries(imGuiDatePicker PUBLIC imgui)

target_compile_options(imGuiDatePicker PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W0>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-w>
)

# libpqxx (vcpkg) ----------------------------------------------------------------------------
# GLOBAL нужен, чтобы IMPORTED-таргет был виден в engine/ (соседняя, не дочерняя директория)
find_package(libpqxx CONFIG REQUIRED GLOBAL)

# podofo (vcpkg) -------------------------------------------------------------------------------
find_package(podofo CONFIG REQUIRED GLOBAL)
