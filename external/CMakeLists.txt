include(FetchContent)

# SDL STATIC BUILD
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# SDL3 ---------------------------------------------------------------------------------------
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.2
)
FetchContent_MakeAvailable(SDL3)

# SDL3_ttf ---------------------------------------------------------------------------------------
#FetchContent_Declare(SDL_ttf
#    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
 #   GIT_TAG release-3.2.2
#)
#FetchContent_MakeAvailable(SDL_ttf)


# PoDoFo ------------------------------------------------------------------------------------
if(WIN32)   #динамическая сборка PoDoFo
find_package(PoDoFo CONFIG REQUIRED)
message(STATUS "Found PoDoFo: ${PoDoFo_VERSION}")

#target_compile_definitions(${PROJECT_NAME} PRIVATE PODOFO_STATIC)
#target_link_libraries(${PROJECT_NAME} PRIVATE
    #${PROJECT_SOURCE_DIR}/out/build/x64-Debug/_deps/podofo-build/src/podofo/podofo.lib
    #${PROJECT_SOURCE_DIR}/out/build/x64-Debug/_deps/podofo-build/src/podofo/private/podofo_private.lib
   # ${PROJECT_SOURCE_DIR}/out/build/x64-Debug/_deps/podofo-build/3rdparty/podofo_3rdparty.lib
    # Добавляем все другие необходимые библиотеки
    #${CMAKE_SOURCE_DIR}/path_to_freetype_lib/freetype.lib
    #${CMAKE_SOURCE_DIR}/path_to_zlib_lib/zlib.lib
    # И так далее для других зависимостей
#)

elseif(UNIX)
set(PODOFO_BUILD_LIB_ONLY TRUE CACHE BOOL "" FORCE)
set(PODOFO_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
FetchContent_Declare(podofo
    GIT_REPOSITORY https://github.com/podofo/podofo.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(podofo)
#Без этой команды #include <podofo/podofo.h> не включается
target_include_directories(${PROJECT_NAME} PRIVATE ${PODOFO_INCLUDE_DIRS})
endif()

# imgui --------------------------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/external/imgui")
add_library(imgui STATIC)
file(GLOB_RECURSE SRC_FILES imgui/*.cpp CMAKE_CONFIGURE_DEPENDS)
target_sources(imgui PRIVATE ${SRC_FILES})
target_link_libraries(imgui PRIVATE SDL3::SDL3)

# imgui docking ----------------------------------------------------------------------------------
#target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/external/imgui-docking")
#add_library(imgui STATIC)
#file(GLOB_RECURSE SRC_FILES imgui-docking/*.cpp CMAKE_CONFIGURE_DEPENDS)
#target_sources(imgui PRIVATE ${SRC_FILES})
#target_link_libraries(imgui PRIVATE SDL3::SDL3)


# toml++ ------------------------------------------------------------------------------------
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)


target_link_libraries(${PROJECT_NAME} PRIVATE
                                    #SDL3_image::SDL3_image
                                    #SDL3_mixer::SDL3_mixer
                                    #SDL3_ttf::SDL3_ttf
                                    imgui
                                    #implot
                                    tomlplusplus::tomlplusplus
                                    podofo::podofo
                                    SDL3::SDL3)   # If using satelite libraries, SDL must be the last item in the list

#target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/pch.hpp)