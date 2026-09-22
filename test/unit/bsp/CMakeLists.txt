add_executable( test_can_tp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_can_tp.c
    ${SRC}/bsp/nucleo_h753zi/can_tp.c
)
target_include_directories( test_can_tp PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../stubs
)
target_link_libraries( test_can_tp PRIVATE
    unity
    test_support
)
add_test( NAME can_tp COMMAND test_can_tp )
