function(target_generate_bin target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:${target}>
            $<TARGET_FILE_DIR:${target}>/${target}.bin
        COMMENT "Generating ${target}.bin"
    )
endfunction()

function(target_generate_hex target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:${target}>
            $<TARGET_FILE_DIR:${target}>/${target}.hex
        COMMENT "Generating ${target}.hex"
    )
endfunction()

function(target_print_size target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${target}>
        COMMENT "Firmware size:"
    )
endfunction()
