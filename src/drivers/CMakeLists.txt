add_subdirectory( button   )
add_subdirectory( display  )
add_subdirectory( uart     )
add_subdirectory( protocol )

add_library( drivers INTERFACE )

target_link_libraries( drivers INTERFACE
    drivers_button
    drivers_display
    drivers_uart
    obd
)
