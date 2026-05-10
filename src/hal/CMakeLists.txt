add_library(hal STATIC
    ${CMAKE_SOURCE_DIR}/src/hal/i2c.c
)

target_include_directories(hal PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(hal PUBLIC
    lib
    cmsis
)