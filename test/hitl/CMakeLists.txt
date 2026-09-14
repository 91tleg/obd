cmake_minimum_required( VERSION 3.20 )
project( hitl C ASM )

set( CMAKE_C_STANDARD 11 )
set( SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src )

if( NOT DEFINED BOARD OR BOARD STREQUAL "" )
    set( BOARD "nucleo_h753zi" CACHE STRING "Target board" )
endif()

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
    set( LINKER_SCRIPT ${SRC}/startup/ram.ld )
else()
    set( LINKER_SCRIPT ${SRC}/startup/flash.ld )
endif()

add_subdirectory( ${SRC}/cmsis    ${CMAKE_BINARY_DIR}/cmsis )
add_subdirectory( ${SRC}/lib      ${CMAKE_BINARY_DIR}/lib )
add_subdirectory( ${SRC}/hal      ${CMAKE_BINARY_DIR}/hal )
add_subdirectory( ${SRC}/bsp      ${CMAKE_BINARY_DIR}/bsp )
add_subdirectory( ${SRC}/drivers  ${CMAKE_BINARY_DIR}/drivers )

add_executable( hitl_can_loopback
    ${SRC}/startup/startup.s
    ${CMAKE_CURRENT_SOURCE_DIR}/main_hitl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_can_loopback.c
    ${SRC}/lib/log/log.c
    ${SRC}/lib/string/str.c
)

target_include_directories( hitl_can_loopback PRIVATE
    ${SRC}
)

target_compile_definitions( hitl_can_loopback PRIVATE
    STM32H753xx
    LOG_ENABLED
)

target_compile_options( hitl_can_loopback PRIVATE
    -Wall
    -Wextra
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

target_link_options( hitl_can_loopback PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${CMAKE_BINARY_DIR}/hitl_can_loopback.map
    -Wl,-u,exit_run0_mode
    -Wl,-u,system_init
    -Wl,-u,main
    -Wl,-u,board_init
    -Wl,--start-group
        $<TARGET_FILE:drivers_button>
        $<TARGET_FILE:drivers_uart>
        $<TARGET_FILE:platform_${BOARD}>
        $<TARGET_FILE:bsp_${BOARD}>
        $<TARGET_FILE:hal>
        $<TARGET_FILE:lib>
    -Wl,--end-group
    -nostdlib
)

target_link_libraries( hitl_can_loopback PRIVATE cmsis )

add_dependencies( hitl_can_loopback
    drivers_button
    drivers_uart
    platform_${BOARD}
    bsp_${BOARD}
    hal
    lib
)

set_target_properties( hitl_can_loopback PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

include( ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/flash.cmake )
target_generate_bin( hitl_can_loopback )
target_generate_hex( hitl_can_loopback )
target_print_size(   hitl_can_loopback )
