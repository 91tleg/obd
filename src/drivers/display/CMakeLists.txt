if( DISPLAY STREQUAL "st7789" )
    add_library( drivers_display STATIC
        ${CMAKE_SOURCE_DIR}/src/drivers/display/st7789/st7789.c
        ${CMAKE_SOURCE_DIR}/src/drivers/display/st7789/font.c
    )
elseif( DISPLAY STREQUAL "hd44780" )
    add_library( drivers_display STATIC
        ${CMAKE_SOURCE_DIR}/src/drivers/display/hd44780/hd44780.c
    )
else()
    message( FATAL_ERROR "DISPLAY must be 'st7789' or 'hd44780', got '${DISPLAY}'" )
endif()
target_include_directories( drivers_display PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries( drivers_display PUBLIC
    lib
)
