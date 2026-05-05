#include "str.h"
#include <stddef.h>

void mem_copy( void * p_dst, const void * p_src, uint32_t len )
{
    if( ( p_dst != NULL ) && ( p_src != NULL ) )
    {
        uint8_t * d = ( uint8_t * )p_dst;
        const uint8_t * s = ( const uint8_t * )p_src;
        uint32_t i = 0U;

        while( i < len )
        {
            d[ i ] = s[ i ];
            i++;
        }
    }
}

void mem_set( void * p_dst, uint8_t val, uint32_t len )
{
    if( p_dst != NULL )
    {
        uint8_t * d = ( uint8_t * )p_dst;
        uint32_t i = 0U;

        while( i < len )
        {
            d[ i ] = val;
            i++;
        }
    }
}

void mem_zero( void * p_dst, uint32_t len )
{
    mem_set( p_dst, 0U, len );
}

bool mem_equal( const void * p_a, const void * p_b, uint32_t len )
{
    bool result = false;

    if( ( p_a != NULL ) && ( p_b != NULL ) )
    {
        
        const uint8_t * a = ( const uint8_t * )p_a;
        const uint8_t * b = ( const uint8_t * )p_b;
        bool match  = true;
        uint32_t i = 0U;

        while( ( i < len ) && match )
        {
            if( a[ i ] != b[ i ] )
            {
                match = false;
            }
            i++;
        }
        result = match;
    }

    return result;
}

uint32_t str_len( const char * p_str )
{
    uint32_t len = 0U;

    if( p_str != NULL )
    {
        while( p_str[ len ] != '\0' )
        {
            len++;
        }
    }

    return len;
}

void str_copy( char * p_dst, const char * p_src, uint32_t max_len )
{
    if( ( p_dst != NULL ) && ( p_src != NULL ) && ( max_len > 0U ) )
    {
        uint32_t i = 0U;

        while( ( i < ( max_len - 1U ) ) && ( p_src[ i ] != '\0' ) )
        {
            p_dst[ i ] = p_src[ i ];
            i++;
        }
        p_dst[ i ] = '\0';
    }
}

bool str_equal( const char * p_a, const char * p_b )
{
    bool result = false;


    if( ( p_a != NULL ) && ( p_b != NULL ) )
    {    
        uint32_t i = 0U;
        bool match = true;

        while( match )
        {
            if( p_a[ i ] != p_b[ i ] )
            {
                match = false;
            }
            else if( p_a[ i ] == '\0' )
            {
                /* both hit null at same position */
                break;
            }
            else
            {
                i++;
            }
        }
        result = match;
    }

    return result;
}

void str_concat( char * p_dst, const char * p_src, uint32_t max_len )
{
    if( ( p_dst != NULL ) && ( p_src != NULL ) && ( max_len > 0U ) )
    {
        uint32_t dst_len = str_len( p_dst );

        if( dst_len < max_len )
        {
            str_copy( &p_dst[ dst_len ], p_src, max_len - dst_len );
        }
    }
}

uint32_t u32_to_str( uint32_t val, char * p_buf, uint32_t buf_len )
{
    uint32_t result = 0U;

    if( ( p_buf != NULL ) && ( buf_len > 1U ) )
    {
        uint32_t remaining = val;

        if( remaining == 0U )
        {
            p_buf[ 0U ] = '0';
            p_buf[ 1U ] = '\0';
            result = 1U;
        }
        else
        {
            char tmp[ 10U ] = { 0 };  /* max uint32 = 4294967295 */
            uint32_t tmp_idx = 0U;
            uint32_t i = 0U;

            /* build digits in reverse */
            while( remaining > 0U )
            {
                tmp[ tmp_idx ] = ( char )( '0' + ( remaining % 10U ) );
                remaining /= 10U;
                tmp_idx++;
            }

            /* reverse into output buffer */
            while( ( i < tmp_idx ) && ( i < ( buf_len - 1U ) ) )
            {
                p_buf[ i ] = tmp[ tmp_idx - 1U - i ];
                i++;
            }

            p_buf[ i ] = '\0';
            result = i;
        }
    }

    return result;
}

uint32_t i32_to_str( int32_t val, char * p_buf, uint32_t buf_len )
{
    uint32_t result = 0U;

    if( ( p_buf != NULL ) && ( buf_len > 1U ) )
    {    
        uint32_t abs_val = 0U;
        uint32_t offset = 0U;

        if( val < 0 )
        {
            p_buf[ 0U ] = '-';
            offset = 1U;
            abs_val = ( uint32_t )( -val );
        }
        else
        {
            abs_val = ( uint32_t )val;
        }

        result = offset + u32_to_str( abs_val, &p_buf[ offset ],
                                      buf_len - offset );
    }

    return result;
}

uint32_t u8_to_hex( uint8_t val, char * p_buf )
{
    uint32_t result = 0U;

    if( p_buf != NULL )
    {
        static const char s_hex[] = "0123456789ABCDEF";
        p_buf[ 0U ] = s_hex[ ( val >> 4U ) & 0x0FU ];
        p_buf[ 1U ] = s_hex[ val & 0x0FU ];
        p_buf[ 2U ] = '\0';
        result = 2U;
    }

    return result;
}
