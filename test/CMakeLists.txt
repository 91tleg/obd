cmake_minimum_required( VERSION 3.16.0 )
project( Tests LANGUAGES C )
set( CMAKE_C_STANDARD 11 )
set( CMAKE_C_STANDARD_REQUIRED ON )
enable_testing()

if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unity/CMakeLists.txt" )
    add_subdirectory( unity )
else()
    message( FATAL_ERROR "Unity submodule not found" )
endif()

# ASan, UBSan
set( SAN_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAN_FLAGS}" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SAN_FLAGS}" )

set( SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../src" )

set( COMMON_TEST_SOURCES
     "${SRC_ROOT}/lib/string/str.c"
)

add_compile_definitions( LOG_ENABLED )

function( create_unity_test TEST_NAME SRC_FILES INCLUDE_DIRS )
    add_executable( ${TEST_NAME} ${SRC_FILES} ${COMMON_TEST_SOURCES} )
    target_include_directories( ${TEST_NAME} PRIVATE ${INCLUDE_DIRS} )
    target_link_libraries( ${TEST_NAME} PRIVATE unity )
    add_test( NAME ${TEST_NAME} COMMAND ${TEST_NAME} )
endfunction()

# Mirrors src/ layout: test/lib/, test/drivers/*, test/app/
file( GLOB_RECURSE TEST_FILES
     "${CMAKE_CURRENT_SOURCE_DIR}/lib/test_*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/app/test_*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/drivers/*/test_*.c"
)

set( APP_INCLUDE_DIRS
     "${SRC_ROOT}"
     "${SRC_ROOT}/lib/math"
     "${SRC_ROOT}/lib/string"
     "${SRC_ROOT}/lib/time"
     "${SRC_ROOT}/bsp"
     "${SRC_ROOT}/hal"
)

set( HEADER_ONLY_MODULES
     ema
)

foreach( TEST_FILE ${TEST_FILES} )
    get_filename_component( TEST_FILENAME ${TEST_FILE} NAME_WE )
    string( REPLACE "test_" "" MODULE_NAME ${TEST_FILENAME} )

    list( FIND HEADER_ONLY_MODULES ${MODULE_NAME} IS_HEADER_ONLY )

    if( IS_HEADER_ONLY GREATER -1 )
        create_unity_test(
            ${TEST_FILENAME}
            "${TEST_FILE}"
            "${APP_INCLUDE_DIRS}"
        )
    else()
        file( GLOB_RECURSE SRC_CANDIDATE
            "${SRC_ROOT}/${MODULE_NAME}.c"
            "${SRC_ROOT}/*/${MODULE_NAME}.c"
            "${SRC_ROOT}/*/*/${MODULE_NAME}.c"
        )

        list( LENGTH SRC_CANDIDATE SRC_COUNT )

        if( SRC_COUNT EQUAL 0 )
            message( WARNING "No source found for '${MODULE_NAME}', skipping ${TEST_FILENAME}." )
        else()
            if( SRC_COUNT GREATER 1 )
                message( WARNING "Multiple sources found for '${MODULE_NAME}', using first match." )
                list( GET SRC_CANDIDATE 0 SRC_CANDIDATE )
            endif()

            create_unity_test(
                ${TEST_FILENAME}
                "${TEST_FILE};${SRC_CANDIDATE}"
                "${APP_INCLUDE_DIRS}"
            )
        endif()
    endif()

endforeach()
