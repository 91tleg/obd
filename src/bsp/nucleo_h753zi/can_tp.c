/**
 * @file    can_tp.c
 * @brief   ISO 15765-2 CAN transport layer — CAN 2.0B and CAN FD 64-byte.
 *          PCI layout per ISO 15765-2:2016 Table 9.
 */

#include "drivers/protocol/obd/can/can_tp.h"
#include <stddef.h>
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/can.h"
#include "lib/time/delay.h"

/* PCI byte masks and type codes */
#define CAN_TP_PCI_TYPE_MASK   ( 0xF0U )
#define CAN_TP_PCI_LEN_MASK    ( 0x0FU )
#define CAN_TP_PCI_SF          ( 0x00U )
#define CAN_TP_PCI_FF          ( 0x10U )
#define CAN_TP_PCI_CF          ( 0x20U )
#define CAN_TP_PCI_FC          ( 0x30U )

/* Flow control flags */
#define CAN_TP_FC_CTS          ( 0x00U )
#define CAN_TP_FC_WAIT         ( 0x01U )
#define CAN_TP_FC_OVFLW        ( 0x02U )

/* Classic CAN 2.0B (CAN_DL = 8) payload sizes */
#define CAN_TP_SF_MAX_DATA     ( 7U )   /* SF: 1 PCI byte  + 7 data       */
#define CAN_TP_FF_DATA_BYTES   ( 6U )   /* FF: 2 PCI bytes + 6 data       */
#define CAN_TP_CF_DATA_BYTES   ( 7U )   /* CF: 1 PCI byte  + 7 data       */

/*
 * SingleFrame (CAN_DL > 8):
 *   Byte #1 = 0x00, Byte #2 = SF_DL, 2 PCI bytes: 62 data bytes
 */
#define CAN_TP_FD_SF_MAX_DATA     ( 62U )

/*
 * FirstFrame (FF_DL ≤ 4095):
 *   Byte #1 = 0x1x, Byte #2 = FF_DL_low, 2 PCI bytes: 62 data bytes
 *
 * FirstFrame (FF_DL > 4095):
 *   Byte #1 = 0x10, Byte #2 = 0x00, Bytes #3-#6 = FF_DL, 6 PCI bytes: 58 data bytes
 */
#define CAN_TP_FD_FF_DATA_STD     ( 62U )   /* standard FF (2-byte header) */
#define CAN_TP_FD_FF_DATA_ESC     ( 58U )   /* escape FF   (6-byte header) */

/*
 * ConsecutiveFrame:
 *   Byte #1 = 0x2N, 1 PCI byte: 63 data bytes in a 64-byte FD frame
 */
#define CAN_TP_FD_CF_DATA_BYTES   ( 63U )

/* Escape sequence threshold*/
#define CAN_TP_FF_DL_ESC_THRESHOLD  ( 4095U )

/* Frame sizes */
#define CAN_TP_FD_FRAME_LEN    ( 64U )

/* Timeouts */
#define CAN_TP_RECV_TIMEOUT_MS ( 25U )
#define CAN_TP_FC_TIMEOUT_MS   ( 250U )

#define CAN_FD_DLC_12   ( 9U  )
#define CAN_FD_DLC_16   ( 10U )
#define CAN_FD_DLC_20   ( 11U )
#define CAN_FD_DLC_24   ( 12U )
#define CAN_FD_DLC_32   ( 13U )
#define CAN_FD_DLC_48   ( 14U )
#define CAN_FD_DLC_64   ( 15U )

static bool is_obd_response( uint32_t id )
{
    return ( ( id >= CAN_TP_OBD_RESP_BASE ) &&
             ( id <= CAN_TP_OBD_RESP_MAX ) );
}

static result_t send_fc( uint8_t flag,
                         uint8_t block_size,
                         uint8_t st_min,
                         bool    use_fd )
{
    can_frame_t fc;
    uint32_t frame_len = use_fd ? CAN_TP_FD_FRAME_LEN : CAN_FRAME_DATA_LEN;

    fc.id  = CAN_TP_OBD_REQ_ID;
    fc.dlc = use_fd ? CAN_FD_DLC_64 : CAN_FRAME_DATA_LEN;
    fc.fd  = use_fd;
    fc.brs = false;   /* no BRS for OBD */

    fc.data[ 0U ] = CAN_TP_PCI_FC | ( flag & CAN_TP_PCI_LEN_MASK );
    fc.data[ 1U ] = block_size;
    fc.data[ 2U ] = st_min;

    for( uint32_t i = 3U; i < frame_len; ++i )
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
        ctx->rx_len      = 0U;
        ctx->block_size  = 0U;
        ctx->st_min_ms   = 0U;
        ctx->sn_expected = 1U;
        ctx->use_fd      = false;
        result = RES_OK;
    }

    return result;
}

result_t can_tp_send( can_tp_ctx_t * ctx,
                      uint32_t req_id,
                      uint8_t const * payload,
                      uint32_t len )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( ctx != NULL ) && ( payload != NULL ) && ( len > 0U ) )
    {
        bool fd = ctx->use_fd;
        uint32_t sf_max    = fd ? CAN_TP_FD_SF_MAX_DATA   : CAN_TP_SF_MAX_DATA;
        uint32_t cf_data   = fd ? CAN_TP_FD_CF_DATA_BYTES : CAN_TP_CF_DATA_BYTES;
        uint32_t frame_len = fd ? CAN_TP_FD_FRAME_LEN     : CAN_FRAME_DATA_LEN;
        uint8_t  dlc_val   = fd ? CAN_FD_DLC_64           : CAN_FRAME_DATA_LEN;

        can_frame_t frame;
        frame.id  = req_id;
        frame.dlc = dlc_val;
        frame.fd  = fd;
        frame.brs = false;

        if( len <= sf_max )
        {
            uint32_t pci_bytes;

            if( fd )
            {
                /* Extended SF — Byte#1=0x00, Byte#2=SF_DL */
                frame.data[ 0U ] = 0x00U;
                frame.data[ 1U ] = ( uint8_t )( len & 0xFFU );
                pci_bytes = 2U;
            }
            else
            {
                /* Classic SF — Byte#1 = [0000|SF_DL] */
                frame.data[ 0U ] = CAN_TP_PCI_SF | ( len & CAN_TP_PCI_LEN_MASK );
                pci_bytes = 1U;
            }

            for( uint32_t i = 0U; i < ( frame_len - pci_bytes ); ++i )
            {
                frame.data[ pci_bytes + i ] =
                    ( i < len ) ? payload[ i ] : 0x00U;
            }

            result = can_send( BSP_CAN, &frame );
        }
        else
        {
            uint32_t offset;
            uint32_t ff_hdr_bytes;
            uint32_t ff_data_bytes;

            if( fd && ( len > CAN_TP_FF_DL_ESC_THRESHOLD ) )
            {
                /* Escape sequence FF */
                frame.data[ 0U ] = CAN_TP_PCI_FF;          /* 0x10 */
                frame.data[ 1U ] = 0x00U;                   /* escape marker */
                frame.data[ 2U ] = ( uint8_t )( ( len >> 24U ) & 0xFFU );
                frame.data[ 3U ] = ( uint8_t )( ( len >> 16U ) & 0xFFU );
                frame.data[ 4U ] = ( uint8_t )( ( len >>  8U ) & 0xFFU );
                frame.data[ 5U ] = ( uint8_t )( ( len        ) & 0xFFU );
                ff_hdr_bytes  = 6U;
                ff_data_bytes = CAN_TP_FD_FF_DATA_ESC;      /* 58 */
            }
            else
            {
                /* Standard FF — 12-bit length */
                frame.data[ 0U ] = CAN_TP_PCI_FF |
                    ( ( uint8_t )( ( len >> 8U ) & CAN_TP_PCI_LEN_MASK ) );
                frame.data[ 1U ] = ( uint8_t )( len & 0xFFU );
                ff_hdr_bytes  = 2U;
                ff_data_bytes = fd ? CAN_TP_FD_FF_DATA_STD  /* 62 */
                                   : CAN_TP_FF_DATA_BYTES;  /* 6  */
            }

            for( uint32_t i = 0U; i < ff_data_bytes; ++i )
            {
                frame.data[ ff_hdr_bytes + i ] = payload[ i ];
            }
            offset = ff_data_bytes;

            /* Pad unused bytes in frame to zero */
            for( uint32_t i = ff_hdr_bytes + ff_data_bytes; i < frame_len; ++i )
            {
                frame.data[ i ] = 0x00U;
            }

            result = can_send( BSP_CAN, &frame );

            /* Wait for Flow Control */
            if( RES_IS_OK( result ) )
            {
                can_frame_t fc_frame;
                result = can_recv( BSP_CAN, &fc_frame, CAN_TP_FC_TIMEOUT_MS );

                if( RES_IS_OK( result ) )
                {
                    if( ( fc_frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK ) != CAN_TP_PCI_FC )
                    {
                        result = RES_ERR_BUS;
                    }
                    else if( ( fc_frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK ) != CAN_TP_FC_CTS )
                    {
                        result = RES_ERR_BUS;
                    }
                }
            }

            /* Consecutive Frames */
            if( RES_IS_OK( result ) )
            {
                uint8_t  sn        = 1U;
                uint32_t remaining = len - offset;

                while( ( offset < len ) && ( RES_IS_OK( result ) ) )
                {
                    uint32_t chunk = ( remaining > cf_data ) ? cf_data : remaining;

                    if( ctx->st_min_ms > 0U )
                    {
                        delay_ms( ctx->st_min_ms );
                    }

                    frame.id  = req_id;
                    frame.dlc = dlc_val;
                    frame.fd  = fd;
                    frame.brs = false;

                    /* Table 9 CF: Byte#1 = [0010|SN] */
                    frame.data[ 0U ] = CAN_TP_PCI_CF | ( sn & 0x0FU );

                    for( uint32_t i = 0U; i < ( frame_len - 1U ); ++i )
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
        bool fd = ctx->use_fd;
        can_frame_t frame;

        result = can_recv( BSP_CAN, &frame, timeout_ms );

        if( RES_IS_OK( result ) && !is_obd_response( frame.id ) )
        {
            result = RES_ERR_PROTOCOL;
        }

        if( RES_IS_OK( result ) )
        {
            uint8_t pci_type = frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK;

            if( pci_type == CAN_TP_PCI_SF )
            {
                uint32_t sf_len;
                uint32_t payload_start;

                if( fd && ( ( frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK ) == 0U ) )
                {
                    /* Extended SF: lower nibble of byte#1 is 0, length in byte#2 */
                    sf_len        = ( uint32_t )frame.data[ 1U ];
                    payload_start = 2U;
                }
                else
                {
                    /* Classic SF */
                    sf_len        = ( uint32_t )frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK;
                    payload_start = 1U;
                }

                uint32_t copy = ( sf_len < buf_size ) ? sf_len : buf_size;

                for( uint32_t i = 0U; i < copy; ++i )
                {
                    buf[ i ] = frame.data[ payload_start + i ];
                }
                *out_len = copy;
            }
            else if( pci_type == CAN_TP_PCI_FF )
            {
                uint32_t total;
                uint32_t ff_hdr_bytes;
                uint32_t ff_data_bytes;
                uint32_t offset = 0U;

                bool escape = fd &&
                              ( ( frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK ) == 0U ) &&
                              ( frame.data[ 1U ] == 0x00U );

                if( escape )
                {
                    total = ( ( uint32_t )frame.data[ 2U ] << 24U ) |
                            ( ( uint32_t )frame.data[ 3U ] << 16U ) |
                            ( ( uint32_t )frame.data[ 4U ] <<  8U ) |
                            ( ( uint32_t )frame.data[ 5U ]        );
                    ff_hdr_bytes  = 6U;
                    ff_data_bytes = CAN_TP_FD_FF_DATA_ESC;   /* 58 */
                }
                else
                {
                    total = ( ( ( uint32_t )frame.data[ 0U ] & CAN_TP_PCI_LEN_MASK ) << 8U ) |
                              ( uint32_t )frame.data[ 1U ];
                    ff_hdr_bytes  = 2U;
                    ff_data_bytes = fd ? CAN_TP_FD_FF_DATA_STD  /* 62 */
                                       : CAN_TP_FF_DATA_BYTES;  /* 6  */
                }

                for( uint32_t i = 0U; i < ff_data_bytes; ++i )
                {
                    if( offset < buf_size )
                    {
                        buf[ offset ] = frame.data[ ff_hdr_bytes + i ];
                        ++offset;
                    }
                }

                result = send_fc( CAN_TP_FC_CTS, ctx->block_size,
                                  ctx->st_min_ms, fd );

                ctx->sn_expected = 1U;

                /* Consecutive Frames */
                while( ( offset < total ) && ( RES_IS_OK( result ) ) )
                {
                    result = can_recv( BSP_CAN, &frame, CAN_TP_RECV_TIMEOUT_MS );

                    if( RES_IS_OK( result ) )
                    {
                        uint8_t cf_pci = frame.data[ 0U ] & CAN_TP_PCI_TYPE_MASK;
                        uint8_t cf_sn  = frame.data[ 0U ] & 0x0FU;

                        if( cf_pci != CAN_TP_PCI_CF )
                        {
                            result = RES_ERR_PROTOCOL;
                        }
                        else if( cf_sn != ( ctx->sn_expected & 0x0FU ) )
                        {
                            result = RES_ERR_PROTOCOL;
                        }
                        else
                        {
                            uint32_t remaining = total - offset;
                            uint32_t cf_data = fd ? CAN_TP_FD_CF_DATA_BYTES
                                                  : CAN_TP_CF_DATA_BYTES;
                            uint32_t chunk = ( remaining < cf_data )
                                             ? remaining : cf_data;

                            for( uint32_t i = 0U; i < chunk; ++i )
                            {
                                if( offset < buf_size )
                                {
                                    buf[ offset ] = frame.data[ i + 1U ];
                                    ++offset;
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
