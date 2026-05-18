file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    source/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    imgui
    implot
    sqlite3
    tomlplusplus::tomlplusplus
    SDL3::SDL3-static
    resources
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
    )
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY
    INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
)
