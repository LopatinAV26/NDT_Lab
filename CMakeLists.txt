cmake_minimum_required(VERSION 4.0.0)
project(SDL3_Engine VERSION 0.1.0 LANGUAGES C CXX)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

    # -Wcast-align
    # -Wcast-qual
    # -Wconversion
    # -Wctor-dtor-privacy
    # -Wenum-compare
    # -Wfloat-equal
    # -Wnon-virtual-dtor
    # -Wold-style-cast
    # -Woverloaded-virtual
    # -Wredundant-decls
    # -Wsign-conversion
    # -Wsign-promo
    message("Debug mode: ${CMAKE_CXX_FLAGS_DEBUG}")

elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -flto")
    message("Release mode: ${CMAKE_CXX_FLAGS_RELEASE}")
else()
    message("else: ${CMAKE_BUILD_TYPE}")
    message("else: ${CMAKE_CXX_FLAGS_RELEASE}")
endif()

add_subdirectory(engine)
add_subdirectory(external EXCLUDE_FROM_ALL)
add_subdirectory(resources)