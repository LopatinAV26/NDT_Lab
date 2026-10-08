# Сервер синхронизации NDT Lab. Собирается отдельно от программы:
#   на сервере (Ubuntu 24.04) - deploy/install.sh, его вызывает deploy/deploy.sh или deploy.ps1;
#   для разработки - локально на Linux, см. README.md
cmake_minimum_required(VERSION 3.25)
project(ndtsync LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)
find_package(nlohmann_json REQUIRED)

# httplib в разных дистрибутивах ставится по-разному: в Arch - с CMake-конфигом,
# в Ubuntu - только с файлом pkg-config
find_package(httplib CONFIG QUIET)
if(httplib_FOUND)
    set(HTTPLIB_TARGET httplib::httplib)
else()
    pkg_check_modules(HTTPLIB REQUIRED IMPORTED_TARGET cpp-httplib)
    set(HTTPLIB_TARGET PkgConfig::HTTPLIB)
endif()

add_executable(ndtsync
    src/main.cpp
    src/env.cpp
    src/schema.cpp
    src/devices.cpp
    src/httpServer.cpp
    src/sync.cpp
)

target_compile_options(ndtsync PRIVATE -Wall -Wextra)
target_link_libraries(ndtsync PRIVATE PkgConfig::PQXX ${HTTPLIB_TARGET} nlohmann_json::nlohmann_json)
