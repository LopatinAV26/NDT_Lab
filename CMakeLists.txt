cmake_minimum_required(VERSION 4.0.0)

# podofo/libpqxx ставятся через vcpkg (тяжёлые системные зависимости: OpenSSL, Freetype,
# LibXml2, libpq). Путь к vcpkg берём из переменной окружения VCPKG_ROOT, чтобы не привязывать
# репозиторий к конкретному расположению на диске.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# CMake's bundled FindTIFF.cmake tries an upstream `find_package(Tiff CONFIG QUIET)`
# before falling back to find_library/find_path. On Arch the system libtiff package
# ships such a config (/usr/lib/cmake/tiff), so podofo ends up linked against the
# dynamic system libtiff (and its shared webp/jbig/zstd deps) instead of vcpkg's
# static one. Disabling that lookup forces the classic find_library path, which
# picks up vcpkg's static libtiff.a.
set(CMAKE_DISABLE_FIND_PACKAGE_Tiff ON CACHE BOOL "")

# На Windows дефолтный триплет vcpkg (x64-windows) линкует зависимости динамически (DLL),
# в отличие от Linux/macOS, где дефолтные триплеты статические. Чтобы получить статическую
# сборку podofo/libpqxx и их зависимостей на Windows, переключаемся на x64-windows-static
# и синхронизируем с ним статический MSVC-рантайм. На Linux/macOS не влияет.
if(WIN32)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET)
        set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
    endif()

    # MSVC (переменная, отличающая cl.exe от clang-cl) определяется только внутри
    # project()/enable_language(), поэтому здесь до project() её ещё не существует -
    # используем WIN32, а не MSVC, чтобы блок реально сработал.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project(NDTLab VERSION 0.1.0 LANGUAGES C CXX)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Задаём как дефолт ДО add_subdirectory(external), чтобы LTO подхватили и свои таргеты
# (imgui, implot, sqlite3), и подтягиваемые через FetchContent (SDL3, SDL3_image).
# На уже собранные vcpkg-библиотеки (podofo, libpqxx и их зависимости) это не влияет -
# они собираются отдельным процессом vcpkg до этого CMake-конфига.
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else()
    message(WARNING "IPO/LTO не поддерживается тулчейном: ${ipo_error}")
endif()

add_subdirectory(external EXCLUDE_FROM_ALL)
add_subdirectory(resources)
add_subdirectory(engine)
