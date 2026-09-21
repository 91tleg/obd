/**
 * @file    hitl_support.c
 * @brief   Glue the freestanding HITL image needs that the app normally
 *          provides.
 */

#include <stddef.h>
#include <stdint.h>
#include "drivers/button/button_callback.h"

/* button.c reports events through this; the HITL has no UI to consume them. */
void btn_event_callback( btn_event_t event )
{
    ( void )event;
}

/*
 * -ffreestanding -nostdlib: GCC still emits calls to memset/memcpy for
 * struct initialisers and copies.
 */
void * memset( void * dst, int c, size_t n )
{
    uint8_t * p = ( uint8_t * )dst;

    while( n-- > 0U )
    {
        *p++ = ( uint8_t )c;
    }

    return dst;
}

void * memcpy( void * dst, void const * src, size_t n )
{
    uint8_t * d = ( uint8_t * )dst;
    uint8_t const * s = ( uint8_t const * )src;

    while( n-- > 0U )
    {
        *d++ = *s++;
    }

    return dst;
}
