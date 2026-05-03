include(${CMAKE_SOURCE_DIR}/cmake/flash.cmake)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/startup/ram.ld)
else()
    set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/src/startup/flash.ld)
endif()

add_executable(firmware
    ${CMAKE_SOURCE_DIR}/src/startup/startup.s
)

target_include_directories(firmware PRIVATE
    ${CMAKE_BINARY_DIR}/generated    # so include bsp/bsp.h finds generated file
    ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory(${CMAKE_SOURCE_DIR}/src/cmsis   ${CMAKE_BINARY_DIR}/cmsis)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/drivers ${CMAKE_BINARY_DIR}/drivers)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/hal     ${CMAKE_BINARY_DIR}/hal)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/bsp     ${CMAKE_BINARY_DIR}/bsp)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/lib     ${CMAKE_BINARY_DIR}/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/app     ${CMAKE_BINARY_DIR}/app)

target_link_libraries(firmware PRIVATE
    cmsis
    hal
    drivers
    lib
    bsp_${BOARD}
    app
)

target_compile_definitions(firmware PRIVATE
    STM32H753xx
)

target_compile_options(firmware PRIVATE
    -Wall
    -Wextra
    -fdata-sections        # allow dead section removal
    -ffunction-sections    # one ELF section per function
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O2>
)

# linker flags
target_link_options(firmware PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections          # strip unused sections
    -Wl,--print-memory-usage   # print RAM/flash usage at link
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    -nostdlib
)

set_target_properties(firmware PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# post-build
target_generate_bin(firmware)
target_generate_hex(firmware)
target_print_size(firmware)
