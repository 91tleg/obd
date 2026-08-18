set( LIB_SRC ${SRC}/lib )

add_executable( test_ring_buffer
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ring_buffer.c
    ${LIB_SRC}/collections/ring_buffer/ring_buffer.c
)
target_include_directories( test_ring_buffer PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries( test_ring_buffer PRIVATE unity )
add_test( NAME ring_buffer COMMAND test_ring_buffer )

add_executable( test_str
    ${CMAKE_CURRENT_SOURCE_DIR}/test_str.c
    ${LIB_SRC}/string/str.c
)
target_include_directories( test_str PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries( test_str PRIVATE unity )
add_test( NAME str COMMAND test_str )

add_executable( test_log
    ${CMAKE_CURRENT_SOURCE_DIR}/test_log.c
    ${LIB_SRC}/log/log.c
    ${LIB_SRC}/string/str.c
)
target_include_directories( test_log PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_compile_definitions( test_log PRIVATE
    LOG_ENABLED
)
target_link_libraries( test_log PRIVATE unity )
add_test( NAME log COMMAND test_log )

add_executable( test_ema
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ema.c
)
target_include_directories( test_ema PRIVATE
    ${SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
target_link_libraries( test_ema PRIVATE unity )
add_test( NAME ema COMMAND test_ema )
