add_library(bsp_nucleo_h753zi STATIC
    clock.c
    hd44780_i2c.c
    i2c.c
    systick.c
    uart_hw.c
)

target_include_directories(bsp_nucleo_h753zi PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bsp      # so "nucleo_h753zi/board.h" resolves
    ${CMAKE_BINARY_DIR}/generated    # so "bsp/bsp.h" resolves
)

target_link_libraries(bsp_nucleo_h753zi PUBLIC
    hal
    cmsis
    drivers
)
