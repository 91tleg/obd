configure_file(
    ${CMAKE_SOURCE_DIR}/src/bsp/bsp.h.in
    ${CMAKE_BINARY_DIR}/generated/bsp/bsp.h
)

add_subdirectory(
    ${CMAKE_SOURCE_DIR}/src/bsp/${BOARD}
    ${CMAKE_BINARY_DIR}/bsp/${BOARD}
)
