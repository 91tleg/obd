add_library( drivers_uart STATIC
    ${CMAKE_SOURCE_DIR}/src/drivers/uart/uart.c
)

target_include_directories( drivers_uart PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries( drivers_uart PUBLIC
    lib
)
