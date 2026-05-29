add_library( obd_kwp2000 STATIC
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/kwp2000/kwp2000.c
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/kwp2000/protocol.c
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/kwp2000/obd_protocol.c
)

target_include_directories( obd_kwp2000 PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries( obd_kwp2000 PUBLIC
    obd_shared
    hal
    lib
)
