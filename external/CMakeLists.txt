include(FetchContent)

# SDL STATIC BUILD
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

# SDL3 ---------------------------------------------------------------------------------------
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.8
)
FetchContent_MakeAvailable(SDL3)

# PoDoFo ------------------------------------------------------------------------------------
if(WIN32)
find_package(PoDoFo CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE PODOFO_STATIC)

elseif(UNIX)
set(PODOFO_BUILD_LIB_ONLY TRUE CACHE BOOL "" FORCE)
set(PODOFO_BUILD_STATIC TRUE CACHE BOOL "" FORCE)

FetchContent_Declare(podofo
    GIT_REPOSITORY https://github.com/podofo/podofo.git
    GIT_TAG 1.0.3
)

FetchContent_MakeAvailable(podofo)

target_include_directories(${PROJECT_NAME} PRIVATE ${PODOFO_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} PRIVATE PODOFO_STATIC)
endif()

# toml++ ------------------------------------------------------------------------------------
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
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

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

target_link_libraries(imgui PUBLIC SDL3::SDL3-static)

# imgui docking ----------------------------------------------------------------------------------
#target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/external/imgui-docking")
#add_library(imgui STATIC)
#file(GLOB_RECURSE SRC_FILES imgui-docking/*.cpp CMAKE_CONFIGURE_DEPENDS)
#target_sources(imgui PRIVATE ${SRC_FILES})
#target_link_libraries(imgui PRIVATE SDL3::SDL3)

# implot -------------------------------------------------------------------------------------
add_library(implot STATIC)

target_sources(implot PRIVATE
    implot/implot.cpp
    implot/implot_items.cpp
    # implot/implot_demo.cpp
)

target_include_directories(implot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/implot")

target_link_libraries(implot PUBLIC imgui)

# sqlite -------------------------------------------------------------------------------------
add_library(sqlite3 STATIC sqlite/sqlite3.c)

target_include_directories(sqlite3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/sqlite")

target_link_libraries(${PROJECT_NAME} PRIVATE
                                    #SDL3_image::SDL3_image
                                    #SDL3_mixer::SDL3_mixer
                                    #SDL3_ttf::SDL3_ttf
                                    imgui
                                    implot
                                    sqlite3
                                    tomlplusplus::tomlplusplus
                                    podofo::podofo
                                    SDL3::SDL3-static)   # If using satelite libraries, SDL must be the last item in the list

#target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/pch.hpp)