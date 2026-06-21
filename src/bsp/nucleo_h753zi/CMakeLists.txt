if( DISPLAY STREQUAL "st7789" )
    set( DISPLAY_PLATFORM_SRC
            st7789_spi.c
            display_st7789.c )
elseif( DISPLAY STREQUAL "hd44780" )
    set( DISPLAY_PLATFORM_SRC
            hd44780_i2c.c
            display_hd44780.c )
endif()

if( OBD_PROTOCOL STREQUAL "kwp2000" )
    set( PROTOCOL_PLATFORM_SRC 
            kwp2000_hw.c )
elseif( OBD_PROTOCOL STREQUAL "can" )
    set( PROTOCOL_PLATFORM_SRC 
            can.c
            can_tp.c )
endif()

add_library( bsp_nucleo_h753zi STATIC
    clock.c
    i2c.c
    systick.c
    wdt.c
)

target_include_directories( bsp_nucleo_h753zi PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bsp
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries( bsp_nucleo_h753zi PUBLIC
    hal
    cmsis
    lib
)

target_link_options( bsp_nucleo_h753zi PRIVATE
    -Wl,--undefined=exit_run0_mode
    -Wl,--undefined=system_init
)

add_library( platform_nucleo_h753zi STATIC
    board.c
    button.c
    uart_hw.c
    uart_debug.c
    uart_obd.c
    log_output.c
    ${DISPLAY_PLATFORM_SRC}
    ${PROTOCOL_PLATFORM_SRC}
)

target_include_directories( platform_nucleo_h753zi PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bsp
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries( platform_nucleo_h753zi PUBLIC
    bsp_nucleo_h753zi
    hal
    lib
    drivers_uart
    drivers_button
    drivers_display
)
