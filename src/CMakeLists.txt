include(${CMAKE_SOURCE_DIR}/cmake/flash.cmake)

if( NOT DEFINED DISPLAY OR DISPLAY STREQUAL "" )
    set( DISPLAY "hd44780" CACHE STRING "Display: st7789 or hd44780" )
endif()
if( NOT DEFINED OBD_PROTOCOL OR OBD_PROTOCOL STREQUAL "" )
    set( OBD_PROTOCOL "can" CACHE STRING "OBD transport: kwp2000 or can" )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/startup/ram.ld )
else()
    set( LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/startup/flash.ld )
endif()

add_executable( firmware
    ${CMAKE_SOURCE_DIR}/src/startup/startup.s
)
target_include_directories( firmware PRIVATE
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory( ${CMAKE_SOURCE_DIR}/src/cmsis   ${CMAKE_BINARY_DIR}/cmsis )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/lib     ${CMAKE_BINARY_DIR}/lib )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/hal     ${CMAKE_BINARY_DIR}/hal )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/drivers ${CMAKE_BINARY_DIR}/drivers )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/bsp     ${CMAKE_BINARY_DIR}/bsp )
add_subdirectory( ${CMAKE_SOURCE_DIR}/src/app     ${CMAKE_BINARY_DIR}/app )

target_link_libraries( firmware PRIVATE
    cmsis
)

if( OBD_PROTOCOL STREQUAL "can" )
    set( OBD_PROTOCOL_LIB obd_can )
else()
    set( OBD_PROTOCOL_LIB obd_kwp2000 )
endif()

target_link_options( firmware PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    -Wl,-u,exit_run0_mode
    -Wl,-u,system_init
    -Wl,-u,main
    -Wl,--start-group
        $<TARGET_FILE:app>
        $<TARGET_FILE:drivers_button>
        $<TARGET_FILE:drivers_display>
        $<TARGET_FILE:drivers_uart>
        $<TARGET_FILE:obd_shared>
        $<TARGET_FILE:${OBD_PROTOCOL_LIB}>
        $<TARGET_FILE:platform_${BOARD}>
        $<TARGET_FILE:bsp_${BOARD}>
        $<TARGET_FILE:hal>
        $<TARGET_FILE:lib>
    -Wl,--end-group
    -nostdlib
)

add_dependencies( firmware
    app
    drivers_button
    drivers_display
    drivers_uart
    obd_shared
    ${OBD_PROTOCOL_LIB}
    platform_${BOARD}
    bsp_${BOARD}
    hal
    lib
)

target_compile_definitions( firmware PRIVATE
    STM32H753xx
)

target_compile_options( firmware PRIVATE
    -Wall
    -Wextra
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

set_target_properties( firmware PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_generate_bin( firmware )
target_generate_hex( firmware )
target_print_size( firmware )
