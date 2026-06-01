/**
 * @file    can_tp.c
 * @brief   ISO 15765-2 CAN transport layer implementation
 */

#include "drivers/protocol/obd/can/can_tp.h"
#include <stddef.h>
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/can.h"
#include "lib/time/delay.h"

/* PCI byte masks */
#define CAN_TP_PCI_TYPE_MASK   ( 0xF0U )
#define CAN_TP_PCI_LEN_MASK    ( 0x0FU )
#define CAN_TP_PCI_SF          ( 0x00U )
#define CAN_TP_PCI_FF          ( 0x10U )
#define CAN_TP_PCI_CF          ( 0x20U )
#define CAN_TP_PCI_FC          ( 0x30U )

/* Flow control flags */
#define CAN_TP_FC_CTS          ( 0x00U )   /* continue to send */
#define CAN_TP_FC_WAIT         ( 0x01U )
#define CAN_TP_FC_OVFLW        ( 0x02U )

/* Frame sizes */
#define CAN_TP_SF_MAX_DATA     ( 7U )      /* SF max payload bytes         */
#define CAN_TP_FF_DATA_BYTES   ( 6U )      /* FF payload bytes             */
#define CAN_TP_CF_DATA_BYTES   ( 7U )      /* CF payload bytes             */

#define CAN_TP_RECV_TIMEOUT_MS ( 25U )     /* inter-frame timeout          */
#define CAN_TP_FC_TIMEOUT_MS   ( 250U )    /* flow control timeout         */

static bool is_obd_response( uint32_t id )
{
    return ( ( id >= CAN_TP_OBD_RESP_BASE ) &&
             ( id <= CAN_TP_OBD_RESP_MAX ) );
}

static result_t send_fc( uint8_t flag,
                         uint8_t block_size,
                         uint8_t st_min )
{
    can_frame_t fc;
    fc.id  = CAN_TP_OBD_REQ_ID;
    fc.dlc = CAN_FRAME_DATA_LEN;
    fc.data[ 0U ] = CAN_TP_PCI_FC | ( flag & CAN_TP_PCI_LEN_MASK );
    fc.data[ 1U ] = block_size;
    fc.data[ 2U ] = st_min;

    for( uint32_t i = 3U; i < CAN_FRAME_DATA_LEN; ++i )
    {
        fc.data[ i ] = 0x00U;
    }

    return can_send( BSP_CAN, &fc );
}

result_t can_tp_init( can_tp_ctx_t * ctx )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ctx != NULL )
    {
        ctx->rx_len     = 0U;
        ctx->block_size = 0U;
        ctx->st_min_ms  = 0U;
        ctx->sn_expected = 1U;
        result = RES_OK;
    }

    return result;
}

result_t can_tp_send( can_tp_ctx_t * ctx,
                      uint32_t req_id,
                      uint8_t const * payload,
                      uint8_t len )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( payload != NULL ) && ( len > 0U ) )
    {
        can_frame_t frame;

        if( len <= CAN_TP_SF_MAX_DATA )
        {
            /* single frame */
            frame.id  = req_id;
            frame.dlc = CAN_FRAME_DATA_LEN;
            frame.data[ 0U ] = CAN_TP_PCI_SF | ( len & CAN_TP_PCI_LEN_MASK );

            for( uint32_t i = 0U; i < CAN_FRAME_DATA_LEN - 1U; ++i )
            {
                frame.data[ i + 1U ] = ( i < ( uint32_t )len )
                                       ? payload[ i ]
                                       : 0x00U;
            }

            result = can_send( BSP_CAN, &frame );
        }
        else
        {
            /* first frame */
            uint32_t offset = 0U;
            uint8_t  sn     = 1U;
            uint32_t remaining;
            can_frame_t fc_frame;

            frame.id  = req_id;
            frame.dlc = CAN_FRAME_DATA_LEN;
            frame.data[ 0U ] = CAN_TP_PCI_FF | ( ( ( uint32_t )len >> 8U ) &
                                               CAN_TP_PCI_LEN_MASK );
            frame.data[ 1U ] = ( uint8_t )( len & 0xFFU );

            for( uint32_t i = 0U; i < CAN_TP_FF_DATA_BYTES; ++i )
            {
                frame.data[ i + 2U ] = payload[ i ];
            }

            offset = CAN_TP_FF_DATA_BYTES;
            result = can_send( BSP_CAN, &frame );

            /* wait for flow control */
            if( RES_IS_OK( result ) )
            {
                result = can_recv( BSP_CAN, &fc_frame, CAN_TP_FC_TIMEOUT_MS );
            }

            if( RES_IS_OK( result ) )
            {
                if( ( fc_frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK ) != CAN_TP_PCI_FC )
                {
                    result = RES_ERR_BUS;
                }
                else if( ( fc_frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK ) !=
                         CAN_TP_FC_CTS )
                {
                    result = RES_ERR_BUS;
                }
                else
                {
                    /* consecutive frames */
                    remaining = ( uint32_t )len - offset;

                    while( ( offset < ( uint32_t )len ) &&
                           ( RES_IS_OK( result ) ) )
                    {
                        uint32_t chunk = ( remaining > CAN_TP_CF_DATA_BYTES )
                                         ? CAN_TP_CF_DATA_BYTES
                                         : remaining;

                        if( ctx->st_min_ms > 0U )
                        {
                            delay_ms( ctx->st_min_ms );
                        }

                        frame.id  = req_id;
                        frame.dlc = CAN_FRAME_DATA_LEN;
                        frame.data[ 0U ] = CAN_TP_PCI_CF | ( sn & 0x0FU );

                        for( uint32_t i = 0U; i < CAN_TP_CF_DATA_BYTES; ++i )
                        {
                            frame.data[ i + 1U ] =
                                ( i < chunk ) ? payload[ offset + i ] : 0x00U;
                        }

                        result = can_send( BSP_CAN, &frame );
                        offset    += chunk;
                        remaining -= chunk;
                        sn = ( uint8_t )( ( sn + 1U ) & 0x0FU );
                    }
                }
            }
        }
    }

    return result;
}

result_t can_tp_recv( can_tp_ctx_t * ctx,
                      uint8_t * buf,
                      uint32_t buf_size,
                      uint32_t * out_len,
                      uint32_t timeout_ms )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( buf != NULL ) &&
        ( out_len != NULL ) && ( buf_size > 0U ) )
    {
        can_frame_t frame;

        result = can_recv( BSP_CAN, &frame, timeout_ms );

        if( ( RES_IS_OK( result ) ) && ( !is_obd_response( frame.id ) ) )
        {
            result = RES_ERR_TIMEOUT;
        }

        if( RES_IS_OK( result ) )
        {
            uint8_t pci_type = frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK;

            if( pci_type == CAN_TP_PCI_SF )
            {
                /* single frame */
                uint32_t sf_len = ( uint32_t )frame.data[ 0U ] &
                                  CAN_TP_PCI_LEN_MASK;
                uint32_t copy = ( sf_len < buf_size ) ? sf_len : buf_size;

                for( uint32_t i = 0U; i < copy; ++i )
                {
                    buf[ i ] = frame.data[ i + 1U ];
                }

                *out_len = copy;
            }
            else if( pci_type == CAN_TP_PCI_FF )
            {
                /* first frame */
                uint32_t total = ( ( ( uint32_t )frame.data[ 0U ] &
                                     CAN_TP_PCI_LEN_MASK ) << 8U ) |
                                     ( uint32_t )frame.data[ 1U ];
                uint32_t offset = 0U;

                for( uint32_t i = 0U; i < CAN_TP_FF_DATA_BYTES; ++i )
                {
                    if( offset < buf_size )
                    {
                        buf[ offset ] = frame.data[ i + 2U ];
                        ++offset;
                    }
                }

                /* send flow control — continue to send */
                result = send_fc( CAN_TP_FC_CTS,
                                  ctx->block_size,
                                  ctx->st_min_ms );

                ctx->sn_expected = 1U;

                /* receive consecutive frames */
                while( ( offset < total ) && ( RES_IS_OK( result ) ) )
                {
                    result = can_recv( BSP_CAN, &frame,
                                       CAN_TP_RECV_TIMEOUT_MS );

                    if( RES_IS_OK( result ) )
                    {
                        uint8_t cf_pci = frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK;
                        uint8_t cf_sn  = frame.data[ 0U ] & 0x0FU;

                        if( cf_pci != CAN_TP_PCI_CF )
                        {
                            result = RES_ERR_BUS;
                        }
                        else if( cf_sn != ( ctx->sn_expected & 0x0FU ) )
                        {
                            result = RES_ERR_BUS;
                        }
                        else
                        {
                            for( uint32_t i = 0U; i < CAN_TP_CF_DATA_BYTES; ++i )
                            {
                                if( offset < buf_size )
                                {
                                    buf[ offset ] = frame.data[ i + 1U ];
                                    offset++;
                                }
                            }

                            ctx->sn_expected = ( uint8_t )
                                ( ( ctx->sn_expected + 1U ) & 0x0FU );
                        }
                    }
                }

                *out_len = ( offset < buf_size ) ? offset : buf_size;
            }
            else
            {
                /* unexpected frame type */
                result = RES_ERR_BUS;
            }
        }
    }

    return result;
}
