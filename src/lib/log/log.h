/**
 * @file    log.h
 * @brief   Lightweight logging — no variadics, no function pointer.
 *
 *          Output is routed via log_output.h.
 *          Compiled out entirely when LOG_ENABLED is not defined.
 */

#ifndef LIB_LOG_LOG_H
#define LIB_LOG_LOG_H

#include <stdint.h>

typedef enum
{
    LOG_NONE    = 0U,
    LOG_ERROR   = 1U,
    LOG_WARN    = 2U,
    LOG_INFO    = 3U,
    LOG_DEBUG   = 4U,
    LOG_VERBOSE = 5U,
} log_level_t;

#define LOG_MAX_MSG_LEN  ( 128U )

#if defined( LOG_ENABLED )

extern log_level_t g_log_level;

void log_set_level( log_level_t level );

void log_write( log_level_t level, const char * tag,
                const char * msg );
void log_write_u32( log_level_t level, const char * tag,
                    const char * msg, uint32_t val );
void log_write_i32( log_level_t level, const char * tag,
                    const char * msg, int32_t val );
void log_write_hex( log_level_t level, const char * tag,
                    const char * msg, uint32_t val );
void log_write_f32( log_level_t level, const char * tag,
                    const char * msg, float val );

#define LOG_WRITE( level, tag, msg ) \
    do { \
        if( (level) <= g_log_level ) \
        { \
            log_write( (level), (tag), (msg) ); \
        } \
    } while( 0 )

#define LOG_WRITE_U32( level, tag, msg, val ) \
    do { \
        if( (level) <= g_log_level ) \
        { \
            log_write_u32( (level), (tag), (msg), (val) ); \
        } \
    } while( 0 )

#define LOG_WRITE_I32( level, tag, msg, val ) \
    do { \
        if( (level) <= g_log_level ) \
        { \
            log_write_i32( (level), (tag), (msg), (val) ); \
        } \
    } while( 0 )

#define LOG_WRITE_HEX( level, tag, msg, val ) \
    do { \
        if( (level) <= g_log_level ) \
        { \
            log_write_hex( (level), (tag), (msg), (val) ); \
        } \
    } while( 0 )

#define LOG_WRITE_F32( level, tag, msg, val ) \
    do { \
        if( (level) <= g_log_level ) \
        { \
            log_write_f32( (level), (tag), (msg), (val) ); \
        } \
    } while( 0 )

#define LOGE( tag, msg )           LOG_WRITE    ( LOG_ERROR,   (tag), (msg) )
#define LOGE_U32( tag, msg, val )  LOG_WRITE_U32( LOG_ERROR,   (tag), (msg), (val) )
#define LOGE_I32( tag, msg, val )  LOG_WRITE_I32( LOG_ERROR,   (tag), (msg), (val) )
#define LOGE_HEX( tag, msg, val )  LOG_WRITE_HEX( LOG_ERROR,   (tag), (msg), (val) )
#define LOGE_F32( tag, msg, val )  LOG_WRITE_F32( LOG_ERROR,   (tag), (msg), (val) )

#define LOGW( tag, msg )           LOG_WRITE    ( LOG_WARN,    (tag), (msg) )
#define LOGW_U32( tag, msg, val )  LOG_WRITE_U32( LOG_WARN,    (tag), (msg), (val) )
#define LOGW_I32( tag, msg, val )  LOG_WRITE_I32( LOG_WARN,    (tag), (msg), (val) )
#define LOGW_HEX( tag, msg, val )  LOG_WRITE_HEX( LOG_WARN,    (tag), (msg), (val) )
#define LOGW_F32( tag, msg, val )  LOG_WRITE_F32( LOG_WARN,    (tag), (msg), (val) )

#define LOGI( tag, msg )           LOG_WRITE    ( LOG_INFO,    (tag), (msg) )
#define LOGI_U32( tag, msg, val )  LOG_WRITE_U32( LOG_INFO,    (tag), (msg), (val) )
#define LOGI_I32( tag, msg, val )  LOG_WRITE_I32( LOG_INFO,    (tag), (msg), (val) )
#define LOGI_HEX( tag, msg, val )  LOG_WRITE_HEX( LOG_INFO,    (tag), (msg), (val) )
#define LOGI_F32( tag, msg, val )  LOG_WRITE_F32( LOG_INFO,    (tag), (msg), (val) )

#define LOGD( tag, msg )           LOG_WRITE    ( LOG_DEBUG,   (tag), (msg) )
#define LOGD_U32( tag, msg, val )  LOG_WRITE_U32( LOG_DEBUG,   (tag), (msg), (val) )
#define LOGD_I32( tag, msg, val )  LOG_WRITE_I32( LOG_DEBUG,   (tag), (msg), (val) )
#define LOGD_HEX( tag, msg, val )  LOG_WRITE_HEX( LOG_DEBUG,   (tag), (msg), (val) )
#define LOGD_F32( tag, msg, val )  LOG_WRITE_F32( LOG_DEBUG,   (tag), (msg), (val) )

#define LOGV( tag, msg )           LOG_WRITE    ( LOG_VERBOSE, (tag), (msg) )
#define LOGV_U32( tag, msg, val )  LOG_WRITE_U32( LOG_VERBOSE, (tag), (msg), (val) )
#define LOGV_I32( tag, msg, val )  LOG_WRITE_I32( LOG_VERBOSE, (tag), (msg), (val) )
#define LOGV_HEX( tag, msg, val )  LOG_WRITE_HEX( LOG_VERBOSE, (tag), (msg), (val) )
#define LOGV_F32( tag, msg, val )  LOG_WRITE_F32( LOG_VERBOSE, (tag), (msg), (val) )

#else

#define log_set_level( level )         ( ( void )0 )

#define LOGE( tag, msg )               ( ( void )0 )
#define LOGE_U32( tag, msg, val )      ( ( void )0 )
#define LOGE_I32( tag, msg, val )      ( ( void )0 )
#define LOGE_HEX( tag, msg, val )      ( ( void )0 )
#define LOGE_F32( tag, msg, val )      ( ( void )0 )

#define LOGW( tag, msg )               ( ( void )0 )
#define LOGW_U32( tag, msg, val )      ( ( void )0 )
#define LOGW_I32( tag, msg, val )      ( ( void )0 )
#define LOGW_HEX( tag, msg, val )      ( ( void )0 )
#define LOGW_F32( tag, msg, val )      ( ( void )0 )

#define LOGI( tag, msg )               ( ( void )0 )
#define LOGI_U32( tag, msg, val )      ( ( void )0 )
#define LOGI_I32( tag, msg, val )      ( ( void )0 )
#define LOGI_HEX( tag, msg, val )      ( ( void )0 )
#define LOGI_F32( tag, msg, val )      ( ( void )0 )

#define LOGD( tag, msg )               ( ( void )0 )
#define LOGD_U32( tag, msg, val )      ( ( void )0 )
#define LOGD_I32( tag, msg, val )      ( ( void )0 )
#define LOGD_HEX( tag, msg, val )      ( ( void )0 )
#define LOGD_F32( tag, msg, val )      ( ( void )0 )

#define LOGV( tag, msg )               ( ( void )0 )
#define LOGV_U32( tag, msg, val )      ( ( void )0 )
#define LOGV_I32( tag, msg, val )      ( ( void )0 )
#define LOGV_HEX( tag, msg, val )      ( ( void )0 )
#define LOGV_F32( tag, msg, val )      ( ( void )0 )

#endif  /* LOG_ENABLED */

#endif  /* LIB_LOG_LOG_H */
