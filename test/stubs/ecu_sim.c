/**
 * @file    ecu_sim.c
 * @brief   OBD ECU simulator stub implementation.
 */

#include "ecu_sim.h"
#include <string.h>

#define SIM_REQ_FUNCTIONAL   ( 0x7DFU )
#define SIM_REQ_PHYSICAL     ( 0x7E0U )
#define SIM_BAD_ID           ( 0x123U )
#define SIM_PAD              ( 0xAAU )
#define SIM_NRC_OUT_OF_RANGE ( 0x31U )
#define SIM_SF_MAX           ( 7U )
#define SIM_FF_DATA          ( 6U )
#define SIM_CF_DATA          ( 7U )
#define SIM_MAX_PAYLOAD      ( ECU_SIM_MAX_DATA + 2U )

typedef struct
{
    uint8_t mode;
    uint8_t pid;
    bool    uses_pid;    /* true = match and echo the request PID   */
    bool    negative;    /* true = reply 7F <mode> <nrc>            */
    uint8_t data[ ECU_SIM_MAX_DATA ];
    uint8_t data_len;
} ecu_entry_t;

static ecu_entry_t     s_entries[ ECU_SIM_MAX_ENTRIES ];
static uint32_t        s_entry_count;
static bool            s_enabled;
static ecu_sim_fault_t s_fault;
static uint32_t        s_resp_id;
static uint32_t        s_request_count;
static uint32_t        s_fc_count;

/* Response waiting for the tester's Flow Control before CFs are released. */
static uint8_t  s_pending[ SIM_MAX_PAYLOAD ];
static uint32_t s_pending_len;
static uint32_t s_pending_offset;
static bool     s_pending_active;

static uint32_t response_id( void )
{
    return ( s_fault == ECU_SIM_FAULT_WRONG_ID ) ? SIM_BAD_ID : s_resp_id;
}

static void queue_frame( uint32_t id, uint8_t const * bytes, uint8_t len )
{
    can_frame_t f;
    memset( &f, 0, sizeof( f ) );
    memset( f.data, SIM_PAD, CAN_FRAME_DATA_LEN );
    f.id  = id;
    f.dlc = CAN_FRAME_DATA_LEN;
    memcpy( f.data, bytes, len );
    stub_queue_recv( &f );
}

static void queue_sf( uint32_t id, uint8_t const * payload, uint8_t len )
{
    uint8_t b[ CAN_FRAME_DATA_LEN ];
    b[ 0 ] = len;   /* PCI 0x0n */
    memcpy( &b[ 1 ], payload, len );
    queue_frame( id, b, ( uint8_t )( len + 1U ) );
}

static void release_cfs( void )
{
    uint8_t sn    = 1U;
    uint32_t nth  = 0U;

    while( s_pending_offset < s_pending_len )
    {
        uint8_t b[ CAN_FRAME_DATA_LEN ];
        uint32_t chunk = s_pending_len - s_pending_offset;
        uint8_t use_sn = sn;

        if( chunk > SIM_CF_DATA )
        {
            chunk = SIM_CF_DATA;
        }

        if( ( s_fault == ECU_SIM_FAULT_BAD_SN ) && ( nth == 1U ) )
        {
            use_sn = ( uint8_t )( ( sn + 3U ) & 0x0FU );
        }

        b[ 0 ] = ( uint8_t )( 0x20U | ( use_sn & 0x0FU ) );
        memcpy( &b[ 1 ], &s_pending[ s_pending_offset ], chunk );

        if( s_fault != ECU_SIM_FAULT_DROP_CFS )
        {
            queue_frame( response_id(), b, ( uint8_t )( chunk + 1U ) );
        }

        s_pending_offset += chunk;
        sn = ( uint8_t )( ( sn + 1U ) & 0x0FU );
        ++nth;
    }

    s_pending_active = false;
}

static void deliver( uint8_t const * payload, uint32_t len )
{
    if( len <= SIM_SF_MAX )
    {
        queue_sf( response_id(), payload, ( uint8_t )len );
    }
    else
    {
        uint8_t b[ CAN_FRAME_DATA_LEN ];

        b[ 0 ] = ( uint8_t )( 0x10U | ( ( len >> 8U ) & 0x0FU ) );
        b[ 1 ] = ( uint8_t )( len & 0xFFU );
        memcpy( &b[ 2 ], payload, SIM_FF_DATA );
        queue_frame( response_id(), b, CAN_FRAME_DATA_LEN );

        /* CFs are held back until the tester sends Flow Control. */
        memcpy( s_pending, payload, len );
        s_pending_len    = len;
        s_pending_offset = SIM_FF_DATA;
        s_pending_active = true;
    }
}

void ecu_sim_reset( void )
{
    memset( s_entries, 0, sizeof( s_entries ) );
    s_entry_count    = 0U;
    s_enabled        = false;
    s_fault          = ECU_SIM_FAULT_NONE;
    s_resp_id        = ECU_SIM_DEFAULT_RESP_ID;
    s_request_count  = 0U;
    s_fc_count       = 0U;
    s_pending_active = false;
    s_pending_len    = 0U;
    s_pending_offset = 0U;
}

static ecu_entry_t * find_or_add( uint8_t mode, uint8_t pid, bool uses_pid )
{
    for( uint32_t i = 0U; i < s_entry_count; ++i )
    {
        if( ( s_entries[ i ].mode == mode ) &&
            ( s_entries[ i ].uses_pid == uses_pid ) &&
            ( !uses_pid || ( s_entries[ i ].pid == pid ) ) )
        {
            return &s_entries[ i ];
        }
    }

    if( s_entry_count < ECU_SIM_MAX_ENTRIES )
    {
        return &s_entries[ s_entry_count++ ];
    }

    return NULL;
}

void ecu_sim_register_pid( uint8_t mode,
                           uint8_t pid,
                           uint8_t const * data,
                           uint8_t len )
{
    ecu_entry_t * e = NULL;

    if( ( data != NULL ) && ( len > 0U ) && ( len <= ECU_SIM_MAX_DATA ) )
    {
        e = find_or_add( mode, pid, true );
    }

    if( e != NULL )
    {
        e->mode     = mode;
        e->pid      = pid;
        e->uses_pid = true;
        e->negative = false;
        e->data_len = len;
        memcpy( e->data, data, len );
    }
}

void ecu_sim_register_mode( uint8_t mode, uint8_t const * data, uint8_t len )
{
    ecu_entry_t * e = NULL;

    if( ( len <= ECU_SIM_MAX_DATA ) && ( ( len == 0U ) || ( data != NULL ) ) )
    {
        e = find_or_add( mode, 0U, false );
    }

    if( e != NULL )
    {
        e->mode     = mode;
        e->pid      = 0U;
        e->uses_pid = false;
        e->negative = false;
        e->data_len = len;
        if( len > 0U )
        {
            memcpy( e->data, data, len );
        }
    }
}

void ecu_sim_register_negative( uint8_t mode, uint8_t pid )
{
    ecu_entry_t * e = find_or_add( mode, pid, true );

    if( e != NULL )
    {
        e->mode     = mode;
        e->pid      = pid;
        e->uses_pid = true;
        e->negative = true;
        e->data_len = 0U;
    }
}

void ecu_sim_enable( bool enable )
{
    s_enabled = enable;
}

void ecu_sim_set_fault( ecu_sim_fault_t fault )
{
    s_fault = fault;
}

void ecu_sim_set_response_id( uint32_t id )
{
    s_resp_id = id;
}

void ecu_sim_inject_sf( uint32_t id, uint8_t const * payload, uint8_t payload_len )
{
    if( ( payload != NULL ) && ( payload_len <= SIM_SF_MAX ) )
    {
        queue_sf( id, payload, payload_len );
    }
}

void ecu_sim_inject_raw( uint32_t id, uint8_t const * bytes, uint8_t len )
{
    if( ( bytes != NULL ) && ( len <= CAN_FRAME_DATA_LEN ) )
    {
        queue_frame( id, bytes, len );
    }
}

uint32_t ecu_sim_request_count( void )
{
    return s_request_count;
}

uint32_t ecu_sim_fc_count( void )
{
    return s_fc_count;
}

static void handle_flow_control( can_frame_t const * frame )
{
    if( ( frame->data[ 0 ] & 0x0FU ) == 0x00U )   /* CTS */
    {
        ++s_fc_count;

        if( s_pending_active )
        {
            release_cfs();
        }
    }
}

static void handle_request( can_frame_t const * frame )
{
    uint8_t len = frame->data[ 0 ] & 0x0FU;

    if( len < 1U )
    {
        return;
    }

    uint8_t mode = frame->data[ 1 ];
    uint8_t pid  = ( len >= 2U ) ? frame->data[ 2 ] : 0U;

    ++s_request_count;
    s_pending_active = false;   /* a new request aborts an unfinished response */

    for( uint32_t i = 0U; i < s_entry_count; ++i )
    {
        ecu_entry_t const * e = &s_entries[ i ];
        uint8_t payload[ SIM_MAX_PAYLOAD ];
        uint32_t n = 0U;

        if( ( e->mode != mode ) || ( e->uses_pid && ( e->pid != pid ) ) )
        {
            continue;
        }

        if( e->negative )
        {
            payload[ n++ ] = 0x7FU;
            payload[ n++ ] = mode;
            payload[ n++ ] = SIM_NRC_OUT_OF_RANGE;
        }
        else
        {
            payload[ n++ ] = ( uint8_t )( mode + 0x40U );

            if( e->uses_pid )
            {
                payload[ n++ ] = ( s_fault == ECU_SIM_FAULT_WRONG_PID_ECHO )
                                 ? ( uint8_t )( pid ^ 0xFFU ) : pid;
            }

            if( e->data_len > 0U )
            {
                memcpy( &payload[ n ], e->data, e->data_len );
                n += e->data_len;
            }
        }

        deliver( payload, n );
        return;
    }

    /* No entry: real ECUs stay silent on unsupported functional requests. */
}

void ecu_sim_process( can_frame_t const * frame )
{
    if( ( frame == NULL ) || ( !s_enabled ) )
    {
        return;
    }

    if( ( frame->id != SIM_REQ_FUNCTIONAL ) && ( frame->id != SIM_REQ_PHYSICAL ) )
    {
        return;
    }

    switch( frame->data[ 0 ] & 0xF0U )
    {
        case 0x00U:
            handle_request( frame );
            break;

        case 0x30U:
            handle_flow_control( frame );
            break;

        default:
            break;   /* tester never segments requests in these tests */
    }
}
