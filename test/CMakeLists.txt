cmake_minimum_required( VERSION 3.20 )
project( tests C )

set( CMAKE_C_STANDARD 11 )
set( SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src )

enable_testing()

add_library( unity STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/unity/src/unity.c
)
target_include_directories( unity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/unity/src
)

add_library( test_support STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/can_stub.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs/ecu_sim.c
    ${SRC}/lib/string/str.c
    ${SRC}/drivers/protocol/obd/cmd.c
    ${SRC}/drivers/protocol/obd/decoder.c
)
target_include_directories( test_support PUBLIC
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
)

add_subdirectory( unit )
add_subdirectory( sil )
