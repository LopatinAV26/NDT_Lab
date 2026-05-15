file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    source/*.cpp
    resources/fonts/*.cpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)				# parallelize each target, unless Ninja is the generator
    endif()
endif()