file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    source/*.cpp
)

add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    implot
    sqlite3
    tomlplusplus::tomlplusplus
    #SDL3_image::SDL3_image-static
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
