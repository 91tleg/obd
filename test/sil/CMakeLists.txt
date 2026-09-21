set( SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../src )

add_executable( test_obd_protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/test_obd_protocol.c
    ${SRC}/drivers/protocol/obd/can/protocol.c
    ${SRC}/drivers/protocol/obd/can/obd_protocol.c
    ${SRC}/bsp/nucleo_h753zi/can_tp.c
)
target_include_directories( test_obd_protocol PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../stubs
)
target_link_libraries( test_obd_protocol PRIVATE
    unity
    test_support
)
add_test( NAME sil_obd_protocol COMMAND test_obd_protocol )
