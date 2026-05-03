add_library(cmsis INTERFACE)

target_include_directories(cmsis INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(cmsis INTERFACE
    STM32H753xx
)
