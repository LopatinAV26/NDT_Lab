# Сервер синхронизации NDT Lab. Собирается отдельно от программы - на самом сервере (Ubuntu 24.04):
#   sudo apt install build-essential cmake libpqxx-dev nlohmann-json3-dev libcpp-httplib-dev
#   ./deploy/install.sh
cmake_minimum_required(VERSION 3.25)
project(ndtsync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
pkg_check_modules(HTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)
find_package(nlohmann_json REQUIRED)

add_executable(ndtsync
    src/main.cpp
    src/env.cpp
    src/schema.cpp
    src/devices.cpp
    src/httpServer.cpp
)

target_compile_options(ndtsync PRIVATE -Wall -Wextra)
target_link_libraries(ndtsync PRIVATE PkgConfig::PQXX PkgConfig::HTTPLIB nlohmann_json::nlohmann_json)
