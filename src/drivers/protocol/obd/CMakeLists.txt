add_library( obd_shared STATIC
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/cmd.c
    ${CMAKE_SOURCE_DIR}/src/drivers/protocol/obd/decoder.c
)

target_include_directories( obd_shared PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries( obd_shared PUBLIC
    lib
)

if( NOT DEFINED OBD_PROTOCOL )
    set( OBD_PROTOCOL "kwp2000" CACHE STRING "OBD transport: kwp2000 or can" )
endif()

set_property( CACHE OBD_PROTOCOL PROPERTY STRINGS kwp2000 can )

message( STATUS "OBD protocol: ${OBD_PROTOCOL}" )

if( OBD_PROTOCOL STREQUAL "kwp2000" )
    add_subdirectory( kwp2000 )
    add_library( obd_protocol ALIAS obd_kwp2000 )
elseif( OBD_PROTOCOL STREQUAL "can" )
    add_subdirectory( can )
    add_library( obd_protocol ALIAS obd_can )
else()
    message( FATAL_ERROR "OBD_PROTOCOL must be 'kwp2000' or 'can', got '${OBD_PROTOCOL}'" )
endif()

add_library( obd INTERFACE )
target_link_libraries( obd INTERFACE
    obd_shared
    obd_protocol
)
