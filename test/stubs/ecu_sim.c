/**
 * @file    ecu_sim.c
 * @brief   OBD ECU simulator stub implementation.
 */

#include "ecu_sim.h"
#include <string.h>

typedef struct
{
    uint8_t mode;
    uint8_t pid;
    uint8_t data[ 64U ];
    uint8_t data_len;
    bool    negative;    /* true = send negative response */
} ecu_pid_entry_t;

static ecu_pid_entry_t s_pids[ ECU_SIM_MAX_PIDS ];
static uint32_t        s_pid_count;
static bool            s_enabled;

static void queue_negative( uint8_t mode )
{
    can_frame_t resp;
    memset( &resp, 0, sizeof( resp ) );
    resp.id      = 0x7E8U;
    resp.data[0] = 0x03U;    /* SF, 3 bytes */
    resp.data[1] = 0x7FU;    /* negative response SID */
    resp.data[2] = mode;     /* request SID */
    resp.data[3] = 0x31U;    /* NRC: request out of range */
    stub_queue_recv( &resp );
}

static void queue_positive( uint8_t mode, uint8_t pid,
                             uint8_t const * data, uint8_t data_len )
{
    can_frame_t resp;
    memset( &resp, 0, sizeof( resp ) );
    resp.id = 0x7E8U;

    /*
     * Response layout (SF):
     *   data[0] = SF_DL = data_len + 2 (mode echo + PID echo)
     *   data[1] = mode + 0x40
     *   data[2] = pid
     *   data[3..] = response bytes
     */
    uint8_t total_len = ( uint8_t )( data_len + 2U );

    if( total_len <= 7U )
    {
        /* Single Frame */
        resp.data[0] = total_len;
        resp.data[1] = ( uint8_t )( mode + 0x40U );
        resp.data[2] = pid;
        memcpy( &resp.data[3], data, data_len );
    }
    else
    {
        /*
         * Multi-byte response.
         * For SIL purposes most OBD PIDs fit in SF (≤7 bytes).
         */
        resp.data[0] = 0x10U | ( ( total_len >> 8U ) & 0x0FU );
        resp.data[1] = ( uint8_t )( total_len & 0xFFU );
        resp.data[2] = ( uint8_t )( mode + 0x40U );
        resp.data[3] = pid;
        uint8_t ff_bytes = 4U;   /* bytes already in FF */
        memcpy( &resp.data[4], data,
                ( data_len < 4U ) ? data_len : 4U );
        stub_queue_recv( &resp );

        /* Queue FC-CTS that the sender expects */
        can_frame_t fc;
        memset( &fc, 0, sizeof( fc ) );
        fc.id = 0x7E8U;
        fc.data[0] = 0x30U;   /* FC CTS */
        stub_queue_recv( &fc );

        /* Queue remaining data in CFs */
        uint8_t sn = 1U;
        uint8_t offset = ff_bytes - 2U;   /* bytes of data already sent */

        while( offset < data_len )
        {
            can_frame_t cf;
            memset( &cf, 0, sizeof( cf ) );
            cf.id      = 0x7E8U;
            cf.data[0] = ( uint8_t )( 0x20U | ( sn & 0x0FU ) );
            uint8_t chunk = ( uint8_t )( data_len - offset );
            if( chunk > 7U ) chunk = 7U;
            memcpy( &cf.data[1], &data[offset], chunk );
            stub_queue_recv( &cf );
            offset += chunk;
            sn = ( uint8_t )( ( sn + 1U ) & 0x0FU );
        }

        return;   /* FF path already queued everything */
    }

    stub_queue_recv( &resp );
}

void ecu_sim_reset( void )
{
    memset( s_pids, 0, sizeof( s_pids ) );
    s_pid_count = 0U;
    s_enabled = false;
}

void ecu_sim_register_pid( uint8_t mode,
                           uint8_t pid,
                           uint8_t const * data,
                           uint8_t len )
{
    if( ( s_pid_count < ECU_SIM_MAX_PIDS ) &&
        ( data != NULL ) &&
        ( len > 0U ) && ( len <= 64U ) )
    {
        ecu_pid_entry_t * e = &s_pids[ s_pid_count++ ];
        e->mode     = mode;
        e->pid      = pid;
        e->data_len = len;
        e->negative = false;
        memcpy( e->data, data, len );
    }
}

void ecu_sim_register_negative( uint8_t mode, uint8_t pid )
{
    if( s_pid_count < ECU_SIM_MAX_PIDS )
    {
        ecu_pid_entry_t * e = &s_pids[ s_pid_count++ ];
        e->mode     = mode;
        e->pid      = pid;
        e->data_len = 0U;
        e->negative = true;
    }
}

void ecu_sim_enable( bool enable )
{
    s_enabled = enable;
}

void ecu_sim_process( can_frame_t const * frame )
{
    if( ( frame == NULL ) || ( !s_enabled ) )
    {
        return;
    }

    /*
     * Determine if this is an OBD request.
     * SF request: data[0] = [0000|len], data[1] = mode, data[2] = pid.
     * Functional request ID: 0x7DF.
     */
    if( frame->id != 0x7DFU )
    {
        return;
    }

    uint8_t pci_type = frame->data[0] & 0xF0U;

    if( pci_type != 0x00U )
    {
        return;   /* only handle SF requests in simulator */
    }

    uint8_t mode = frame->data[1];
    uint8_t pid  = frame->data[2];

    for( uint32_t i = 0U; i < s_pid_count; ++i )
    {
        if( ( s_pids[i].mode == mode ) && ( s_pids[i].pid == pid ) )
        {
            if( s_pids[i].negative )
            {
                queue_negative( mode );
            }
            else
            {
                queue_positive( mode, pid,
                                s_pids[i].data,
                                s_pids[i].data_len );
            }
            return;
        }
    }
}
