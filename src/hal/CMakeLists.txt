add_library(hal INTERFACE)

target_include_directories(hal INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(hal INTERFACE cmsis)
