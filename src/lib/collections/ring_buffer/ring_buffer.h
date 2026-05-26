/**
 * @file    ring_buffer.h
 * @brief   Power-of-2 lock-free ring buffer
 *          Single producer / single consumer ISR safe
 */

#ifndef LIB_RING_BUFFER_H
#define LIB_RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t * buf;
    uint32_t size;  /* must be power of 2 for mask */
    volatile uint32_t head;
    volatile uint32_t tail;
} ring_buf_t;

void ring_buf_init( ring_buf_t * rb, uint8_t * buf, uint32_t size );
bool ring_buf_push( ring_buf_t * rb, uint8_t data );
bool ring_buf_pop( ring_buf_t * rb, uint8_t * p_data );
bool ring_buf_peek( const ring_buf_t * rb, uint8_t * p_data );
uint32_t ring_buf_used( const ring_buf_t * rb );
uint32_t ring_buf_free( const ring_buf_t * rb );
bool ring_buf_full( const ring_buf_t * rb );
bool ring_buf_empty( const ring_buf_t * rb );
void ring_buf_clear( ring_buf_t * rb );

#endif /* LIB_RING_BUFFER_H */
