add_library(resources INTERFACE)

target_include_directories(resources INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/fonts
)
