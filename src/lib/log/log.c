#include "log.h"

#if defined( LOG_ENABLED )

#include <stddef.h>
#include "log_output.h"
#include "lib/string/str.h"
#include "lib/core/macros.h"

log_level_t g_log_level = LOG_INFO;   /* extern in header, read by macros */

static const char * const s_level_str[] =
{
    "",    /* LOG_NONE    */
    "E",   /* LOG_ERROR   */
    "W",   /* LOG_WARN    */
    "I",   /* LOG_INFO    */
    "D",   /* LOG_DEBUG   */
    "V",   /* LOG_VERBOSE */
};

#define LEVEL_STR_COUNT  ARRAY_SIZE( s_level_str )

/**
 * Copy at most (buf_len - 1) chars from str into buf.
 * Does NOT null-terminate — caller manages position and termination.
 * Returns number of chars written.
 */
static uint32_t fmt_str( char * buf,
                         uint32_t buf_len,
                         const char * str )
{
    uint32_t i = 0U;

    if( buf_len > 1U )
    {
        uint32_t len = str_len( str );

        while( ( i < len ) && ( i < ( buf_len - 1U ) ) )
        {
            buf[ i ] = str[ i ];
            i++;
        }
    }

    return i;
}

/**
 * Format and emit one log line.
 * Output format: "[L] (tag): msg<detail>\r\n"
 * detail may be NULL.
 */
static void log_format_and_emit( log_level_t level,
                                 const char * tag,
                                 const char * msg,
                                 const char * detail )
{
    char buf[ LOG_MAX_MSG_LEN ];
    uint32_t pos = 0U;
    uint32_t rem;

    /* "[L] " */
    buf[ pos ] = '[';
    pos++;
    pos += fmt_str( &buf[ pos ], sizeof( buf ) - pos,
                    s_level_str[ ( uint32_t )level ] );
    buf[ pos ] = ']';
    pos++;
    buf[ pos ] = ' ';
    pos++;

    /* "(tag): " */
    buf[ pos ] = '(';
    pos++;
    pos += fmt_str( &buf[ pos ], sizeof( buf ) - pos,
                    ( tag != NULL ) ? tag : "?" );
    buf[ pos ] = ')';
    pos++;
    buf[ pos ] = ':';
    pos++;
    buf[ pos ] = ' ';
    pos++;

    /* message */
    rem  = sizeof( buf ) - pos - 2U;    /* reserve 2 for \r\n */
    pos += fmt_str( &buf[ pos ], rem, ( msg != NULL ) ? msg : "" );

    /* optional detail string (pre-formatted by typed write functions) */
    if( detail != NULL )
    {
        rem  = sizeof( buf ) - pos - 2U;
        pos += fmt_str( &buf[ pos ], rem, detail );
    }

    /* line ending */
    if( pos < ( sizeof( buf ) - 2U ) )
    {
        buf[ pos ] = '\r';
        pos++;
        buf[ pos ] = '\n';
        pos++;
    }

    buf[ pos ] = '\0';

    log_write_output( buf, pos );
}

static bool log_should_emit( log_level_t level )
{
    return ( ( level != LOG_NONE ) &&
             ( level <= g_log_level ) &&
             ( ( uint32_t )level < LEVEL_STR_COUNT ) );
}

void log_set_level( log_level_t level )
{
    g_log_level = level;
}

void log_write( log_level_t level,
                const char * tag,
                const char * msg )
{
    if( log_should_emit( level ) )
    {
        log_format_and_emit( level, tag, msg, NULL );
    }
}

void log_write_u32( log_level_t level,
                    const char * tag,
                    const char * msg,
                    uint32_t val )
{
    if( log_should_emit( level ) )
    {
        char detail[ 12U ];
        ( void )u32_to_str( val, detail, sizeof( detail ) );
        log_format_and_emit( level, tag, msg, detail );
    }
}

void log_write_i32( log_level_t level,
                    const char * tag,
                    const char * msg,
                    int32_t val )
{
    if( log_should_emit( level ) )
    {
        char detail[ 13U ];  /* sign + 10 digits + null */
        ( void )i32_to_str( val, detail, sizeof( detail ) );
        log_format_and_emit( level, tag, msg, detail );
    }
}

void log_write_hex( log_level_t level,
                    const char * tag,
                    const char * msg,
                    uint32_t val )
{
    if( log_should_emit( level ) )
    {
        char detail[ 9U ]; /* 8 nibbles + null */
        ( void )u32_to_hex( val, detail, sizeof( detail ) );
        log_format_and_emit( level, tag, msg, detail );
    }
}

void log_write_f32( log_level_t level,
                    const char * tag,
                    const char * msg,
                    float val )
{
    if( log_should_emit( level ) )
    {
        char detail[ 16U ];
        ( void )f32_to_str( val, detail, sizeof( detail ), 2U );
        log_format_and_emit( level, tag, msg, detail );
    }
}

#endif  /* LOG_ENABLED */
