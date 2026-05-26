#include "ring_buffer.h"
#include <stddef.h>

#define MASK( rb )  ( ( ( rb )->size ) - 1U )

void ring_buf_init( ring_buf_t * p_rb, uint8_t * buf, uint32_t size )
{
    if( ( p_rb != NULL ) && ( buf != NULL ) &&
        ( size > 0U ) && ( ( size & ( size - 1U ) ) == 0U ) )
    {
        p_rb->buf  = buf;
        p_rb->size = size;
        p_rb->head = 0U;
        p_rb->tail = 0U;
    }
}

bool ring_buf_push( ring_buf_t * p_rb, uint8_t data )
{
    bool result = false;

    if( ( p_rb != NULL ) && ( !ring_buf_full( p_rb ) ) )
    {
        p_rb->buf[ p_rb->head & MASK( p_rb ) ] = data;
        p_rb->head++;
        result = true;
    }

    return result;
}

bool ring_buf_pop( ring_buf_t * p_rb, uint8_t * p_data )
{
    bool result = false;

    if( ( p_rb != NULL ) && ( p_data != NULL ) && ( !ring_buf_empty( p_rb ) ) )
    {
        *p_data  = p_rb->buf[ p_rb->tail & MASK( p_rb ) ];
        p_rb->tail++;
        result   = true;
    }

    return result;
}

bool ring_buf_peek( const ring_buf_t * p_rb, uint8_t * p_data )
{
    bool result = false;

    if( ( p_rb != NULL ) && ( p_data != NULL ) && ( !ring_buf_empty( p_rb ) ) )
    {
        *p_data = p_rb->buf[ p_rb->tail & MASK( p_rb ) ];
        result  = true;
    }

    return result;
}

uint32_t ring_buf_used( const ring_buf_t * p_rb )
{
    uint32_t used = 0U;

    if( p_rb != NULL )
    {
        uint32_t const head = p_rb->head;
        uint32_t const tail = p_rb->tail;
        used = head - tail;
    }

    return used;
}

uint32_t ring_buf_free( const ring_buf_t * p_rb )
{
    uint32_t free_space = 0U;

    if( p_rb != NULL )
    {
        free_space = p_rb->size - ring_buf_used( p_rb );
    }

    return free_space;
}

bool ring_buf_full( const ring_buf_t * p_rb )
{
    bool result = false;

    if( p_rb != NULL )
    {
        result = ( ring_buf_used( p_rb ) == p_rb->size );
    }

    return result;
}

bool ring_buf_empty( const ring_buf_t * p_rb )
{
    bool result = true;

    if( p_rb != NULL )
    {
        uint32_t const head = p_rb->head;
        uint32_t const tail = p_rb->tail;
        result = ( head == tail );
    }

    return result;
}

void ring_buf_clear( ring_buf_t * p_rb )
{
    if( p_rb != NULL )
    {
        p_rb->head = 0U;
        p_rb->tail = 0U;
    }
}
