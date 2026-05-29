add_library(lib STATIC
    collections/ring_buffer/ring_buffer.c
    string/str.c
    time/delay.c
    log/log.c
)

target_include_directories(lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
