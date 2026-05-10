add_library(lib STATIC
    string/str.c
    time/delay.c
)

target_include_directories(lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
