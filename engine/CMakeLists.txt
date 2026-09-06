file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    source/*.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})

# Иконка .exe в проводнике Windows. resources/app.rc ссылается на
# resources/app.ico (нужно положить туда сам .ico - см. пояснение).
# На других платформах .rc не используется, поэтому подключаем только под Windows.
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/resources/app.rc")
endif()

# Помощники отладчика Visual Studio. Подключаются именно к exe, а не к статической
# библиотеке imgui: .natvis попадает в PDB на этапе линковки, у статической библиотеки
# этого этапа нет. imgui.natvis - читаемое отображение ImVector, ImGuiStorage и прочих
# типов ImGui в окне watch; imgui.natstepfilter - F11 не проваливается внутрь тривиальных
# функций ImGui (файл в проекте подхватывается начиная с VS 2022 17.6).
if(MSVC)
    target_sources(${PROJECT_NAME} PRIVATE
        "${CMAKE_SOURCE_DIR}/external/imgui/imgui.natvis"
        "${CMAKE_SOURCE_DIR}/external/imgui/imgui.natstepfilter"
    )
endif()

# Файлы в source/appWindows/ (settingsWindow, protocolWindow, nomogramWindow,
# reportWindow, defectCreateWindow) инклюдят соседние заголовки из
# source/ (applicationData.hpp, resourceManager.hpp, laboratory.hpp,
# reportBuilder.hpp) без префикса пути - добавляем source/ в include path,
# чтобы такие "#include "applicationData.hpp"" резолвились и из подкаталога.
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/source")

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    implot
    imGuiDatePicker
    sqlite3
    tomlplusplus::tomlplusplus
    stb_image
    SDL3::SDL3-static
    resources
    libpqxx::pqxx
    podofo::podofo
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)

    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wold-style-cast
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wformat=2
        #-O3

        #векторизация
        #-march=native  процессорозависимо
        -march=x86-64-v3 #(процессоры не старше Haswell, ~2013+)
    )
        #отладка оптимизатора
        # -fopt-info-vec есть только у GCC, clang падает с "unknown argument"
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-fopt-info-vec>
        #$<$<CXX_COMPILER_ID:GNU>:-fopt-info-missed>
    )
endif()
