add_library( app STATIC
    main.c
    app_obd.c
    app_display.c
)

target_include_directories( app PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/bsp
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries( app PRIVATE
    lib
    drivers
)
