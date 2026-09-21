# Built from the repo root:  cmake -B build/hitl -DBUILD_HITL=ON
# (src/ CMake files resolve paths via CMAKE_SOURCE_DIR, so this cannot be a
# standalone top-level project.)
set( CMAKE_C_STANDARD 11 )
set( SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src )

# Required by src/drivers and src/bsp (same defaults as src/CMakeLists.txt).
if( NOT DEFINED DISPLAY OR DISPLAY STREQUAL "" )
    set( DISPLAY "hd44780" CACHE STRING "Display: st7789 or hd44780" )
endif()
if( NOT DEFINED OBD_PROTOCOL OR OBD_PROTOCOL STREQUAL "" )
    set( OBD_PROTOCOL "can" CACHE STRING "OBD transport: kwp2000 or can" )
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
    ${CMAKE_CURRENT_SOURCE_DIR}/hitl_support.c
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
    -nostdlib
)

# Via target_link_libraries (not raw link options) so the archives are placed
# after this target's objects and CMake tracks them as relink dependencies.
target_link_libraries( hitl_can_loopback PRIVATE
    cmsis
    -Wl,--start-group
        drivers_button
        drivers_uart
        platform_${BOARD}
        bsp_${BOARD}
        hal
        lib
    -Wl,--end-group
)

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
