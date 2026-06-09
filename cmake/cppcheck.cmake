find_program(CPPCHECK cppcheck)

if( CPPCHECK )
    get_filename_component(CPPCHECK_BIN_DIR ${CPPCHECK} DIRECTORY)
    find_file(MISRA_PY
        NAMES misra.py
        PATHS
            ${CPPCHECK_BIN_DIR}/../share/cppcheck/addons
            /usr/local/share/cppcheck/addons
            /usr/share/cppcheck/addons
            /opt/homebrew/share/cppcheck/addons
        NO_DEFAULT_PATH
    )

    if( NOT MISRA_PY )
        message(WARNING "misra.py not found")
        set(MISRA_ADDON "")
    else()
        message(STATUS "Found misra.py: ${MISRA_PY}")
        set(MISRA_ADDON "--addon=${MISRA_PY}")
    endif()

    add_custom_target(cppcheck
        COMMAND ${CPPCHECK}
            ${MISRA_ADDON}
            --check-level=exhaustive
            --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck-suppressions.txt
            --enable=all
            --error-exitcode=1
            --platform=arm32-wchar_t2
            --suppress=missingIncludeSystem
            -i${CMAKE_SOURCE_DIR}/src/cmsis
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
        COMMENT "Running cppcheck MISRA analysis"
        VERBATIM
    )
else()
    message(WARNING "cppcheck not found")
endif()
