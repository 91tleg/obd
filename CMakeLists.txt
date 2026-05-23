cmake_minimum_required(VERSION 3.24)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(firmware C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BOARD "nucleo_h753zi" CACHE STRING "Target board")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/cppcheck.cmake)

add_subdirectory(src)
