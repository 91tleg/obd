/**
 * @file    str.h
 * @brief   Minimal string and memory utilities
 *          No libc dependency
 */

#ifndef LIB_STR_H
#define LIB_STR_H

#include <stdint.h>
#include <stdbool.h>

void mem_copy( void * p_dst, const void * p_src, uint32_t len );
void mem_set( void * p_dst, uint8_t val, uint32_t len );
bool mem_equal( const void * p_a, const void * p_b, uint32_t len );

uint32_t str_len( const char * p_str );
void str_copy( char * p_dst, const char * p_src, uint32_t max_len );
bool str_equal( const char * p_a, const char * p_b );
void str_concat( char * p_dst, const char * p_src, uint32_t max_len );

uint32_t u32_to_str( uint32_t val, char * p_buf, uint32_t buf_len );
uint32_t i32_to_str( int32_t  val, char * p_buf, uint32_t buf_len );
uint32_t u8_to_hex( uint8_t  val, char * p_buf );

#endif /* LIB_STR_H */
