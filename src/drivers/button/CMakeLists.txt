add_library( drivers_button STATIC
    ${CMAKE_SOURCE_DIR}/src/drivers/button/button.c
)

target_include_directories( drivers_button PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries( drivers_button PUBLIC
    lib
)
