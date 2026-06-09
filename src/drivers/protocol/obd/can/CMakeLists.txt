add_library( obd_can STATIC
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/can/protocol.c
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/can/obd_protocol.c
)

target_include_directories( obd_can PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries( obd_can PUBLIC
    obd_shared
    hal
    lib
)
