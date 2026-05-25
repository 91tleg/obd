#ifndef LIB_CORE_MACROS_H
#define LIB_CORE_MACROS_H

/**
 * Number of elements in a stack-allocated array.
 * Not valid for pointers or dynamically allocated arrays.
 */
#define ARRAY_SIZE( arr )  \
    ( sizeof( arr ) / sizeof( ( arr )[ 0 ] ) )

#endif /* LIB_CORE_MACROS_H */
