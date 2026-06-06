/**
 * @file    can.c
 * @brief   STM32H753 FDCAN HAL implementation. Classic CAN and CAN FD 64-byte.
 *
 * Message RAM layout (per peripheral):
 *
 *   0x000  Standard ID filter list  ( 1 element ×  4 B =   4 B)
 *   0x004  RX FIFO 0                ( 8 elements × 72 B = 576 B)
 *   0x244  TX FIFO                  ( 8 elements × 72 B = 576 B)
 *
 * Each FD element = 2 header words (8 B) + 16 data words (64 B) = 72 bytes.
 * RXESC and TXESC are set to 0x7 to select the 64-byte payload size.
 */

#include "can.h"
#include <stddef.h>
#include "lib/time/delay_tick.h"

#define FDCAN1_RAM_BASE  ( SRAMCAN_BASE )
#define FDCAN2_RAM_BASE  ( SRAMCAN_BASE + 0x0550U )

#define NUM_STD_FILTERS        ( 1U )
#define NUM_RX_FIFO0_ELEMENTS  ( 8U )
#define NUM_TX_FIFO_ELEMENTS   ( 8U )

/*
 * CAN FD 64-byte element layout in message RAM:
 *   2 header words + 16 data words = 18 words = 72 bytes per element.
 */
#define CAN_FD_HEADER_WORDS    ( 2U )
#define CAN_FD_DATA_WORDS      ( 16U )   /* 16 × 4 = 64 bytes */
#define CAN_FD_ELEMENT_WORDS   ( CAN_FD_HEADER_WORDS + CAN_FD_DATA_WORDS )
#define CAN_ELEMENT_SIZE_BYTES ( CAN_FD_ELEMENT_WORDS * 4U )   /* 72 bytes */

#define RAM_OFF_STD_FILTER  ( 0x000U )
#define RAM_OFF_RXF0        ( RAM_OFF_STD_FILTER + ( NUM_STD_FILTERS * 4U ) )
#define RAM_OFF_TXF         ( RAM_OFF_RXF0 + \
                              ( NUM_RX_FIFO0_ELEMENTS * CAN_ELEMENT_SIZE_BYTES ) )

/* RXESC / TXESC: data field size = 111b: 64 bytes */
#define ESC_64_BYTES           ( 0x7U )

#define STD_FILTER_SFEC_RXF0   ( 0x3U )
#define STD_FILTER_SFT_RANGE   ( 0x1U )
#define STD_FILTER_SFEC_Pos    ( 30U )
#define STD_FILTER_SFT_Pos     ( 27U )
#define STD_FILTER_SFID1_Pos   ( 16U )
#define STD_FILTER_SFID2_Pos   (  0U )
#define STD_FILTER_ID_MASK     ( 0x7FFU )

/* TX/RX buffer element word 1 bit positions */
#define TXBE_T1_DLC_Pos        ( 16U )
#define TXBE_T1_BRS_Pos        ( 20U )   /* Bit Rate Switch         */
#define TXBE_T1_FDF_Pos        ( 21U )   /* FD Format               */

/* RX buffer element word 1 bit positions */
#define RXBE_R1_DLC_Pos        ( 16U )
#define RXBE_R1_BRS_Pos        ( 20U )
#define RXBE_R1_FDF_Pos        ( 21U )

/*
 * Convert raw DLC value (0–15) to payload byte count.
 * CAN FD encoding: 0–8 map 1:1; 9–15 map to 12,16,20,24,32,48,64.
 */
static uint32_t dlc_to_bytes( uint8_t dlc )
{
    static const uint8_t fd_dlc_table[ 16U ] =
    {
        0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
        12U, 16U, 20U, 24U, 32U, 48U, 64U
    };
    return ( dlc <= 15U ) ? fd_dlc_table[ dlc ] : 0U;
}

static uint32_t ram_base( FDCAN_GlobalTypeDef const * p_can )
{
    uint32_t base = 0U;

    if( p_can == FDCAN1 )
    {
        base = FDCAN1_RAM_BASE;
    }
    else if( p_can == FDCAN2 )
    {
        base = FDCAN2_RAM_BASE;
    }
    else
    {
        /* invalid */
    }

    return base;
}

static bool timed_out( uint32_t start, uint32_t timeout_ms )
{
    return ( ( delay_get_tick() - start ) >= timeout_ms );
}

result_t can_init( FDCAN_GlobalTypeDef * p_can,
                   can_timing_t timing,
                   can_data_timing_t data_timing,
                   can_filter_t const * filter )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_can != NULL ) && ( filter != NULL ) )
    {
        uint32_t start;
        uint32_t base;
        uint32_t * ram;

        p_can->CCCR |= FDCAN_CCCR_INIT;
        start = delay_get_tick();

        while( ( ( p_can->CCCR & FDCAN_CCCR_INIT ) == 0U ) &&
               ( !timed_out( start, 10U ) ) )
        {
            /* wait for hardware to acknowledge init mode */
        }

        if( ( p_can->CCCR & FDCAN_CCCR_INIT ) == 0U )
        {
            result = RES_ERR_TIMEOUT;
        }
        else
        {
            p_can->CCCR |= FDCAN_CCCR_CCE;

            /* Nominal bit timing */
            p_can->NBTP = timing;

            /*
             * Data-phase bit timing for BRS.
             * If data_timing is 0 the DBTP reset value is used which is
             * harmless — BRS frames simply won't switch rate.
             */
            if( data_timing != 0U )
            {
                p_can->DBTP = data_timing;
            }

            /* Enable FD operation (FDOE) and optionally BRS (BRSE). */
            p_can->CCCR |= FDCAN_CCCR_FDOE;
            if( data_timing != 0U )
            {
                p_can->CCCR |= FDCAN_CCCR_BRSE;
            }

            p_can->SIDFC =
                ( ( ( RAM_OFF_STD_FILTER >> 2U ) << FDCAN_SIDFC_FLSSA_Pos ) &
                    FDCAN_SIDFC_FLSSA_Msk ) |
                ( NUM_STD_FILTERS << FDCAN_SIDFC_LSS_Pos );

            p_can->RXF0C =
                ( ( ( RAM_OFF_RXF0 >> 2U ) << FDCAN_RXF0C_F0SA_Pos ) &
                    FDCAN_RXF0C_F0SA_Msk ) |
                ( NUM_RX_FIFO0_ELEMENTS << FDCAN_RXF0C_F0S_Pos );

            p_can->TXBC =
                ( ( ( RAM_OFF_TXF >> 2U ) << FDCAN_TXBC_TBSA_Pos ) &
                    FDCAN_TXBC_TBSA_Msk ) |
                ( NUM_TX_FIFO_ELEMENTS << FDCAN_TXBC_TFQS_Pos );

            /*
             * RXESC / TXESC: set data field size to 64 bytes (value 0x7).
             * Both RX FIFO 0 (F0DS) and TX buffers (TBDS) must match.
             */
            p_can->RXESC = ( ESC_64_BYTES << FDCAN_RXESC_F0DS_Pos );
            p_can->TXESC = ( ESC_64_BYTES << FDCAN_TXESC_TBDS_Pos );

            base = ram_base( p_can );
            ram  = ( uint32_t * )( base + RAM_OFF_STD_FILTER );

            *ram =
                ( ( STD_FILTER_SFEC_RXF0                 ) << STD_FILTER_SFEC_Pos  ) |
                ( ( STD_FILTER_SFT_RANGE                 ) << STD_FILTER_SFT_Pos   ) |
                ( ( filter->id_high & STD_FILTER_ID_MASK ) << STD_FILTER_SFID1_Pos ) |
                ( ( filter->id_low  & STD_FILTER_ID_MASK ) << STD_FILTER_SFID2_Pos );

            p_can->GFC = ( FDCAN_GFC_ANFE_Msk | FDCAN_GFC_ANFS_Msk );

            p_can->CCCR &= ~( FDCAN_CCCR_CCE | FDCAN_CCCR_INIT );
            start = delay_get_tick();
            while( ( ( p_can->CCCR & FDCAN_CCCR_INIT ) != 0U ) &&
                   ( !timed_out( start, 10U ) ) )
            {
                /* wait for hardware to release init mode */
            }

            result = ( ( p_can->CCCR & FDCAN_CCCR_INIT ) == 0U )
                     ? RES_OK
                     : RES_ERR_TIMEOUT;
        }
    }

    return result;
}

result_t can_send( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t const * frame )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_can != NULL ) && ( frame != NULL ) )
    {
        if( ( p_can->TXFQS & FDCAN_TXFQS_TFQF ) != 0U )
        {
            result = RES_ERR_BUSY;
        }
        else
        {
            uint32_t put_idx =
                ( p_can->TXFQS & FDCAN_TXFQS_TFQPI_Msk ) >> FDCAN_TXFQS_TFQPI_Pos;

            uint32_t   base = ram_base( p_can );
            uint32_t * elem = ( uint32_t * )
                ( base + RAM_OFF_TXF + ( put_idx * CAN_ELEMENT_SIZE_BYTES ) );

            /* T0: standard ID in bits [28:18] */
            elem[ 0U ] = ( frame->id & STD_FILTER_ID_MASK ) << 18U;

            /*
             * T1: DLC [19:16], BRS [20], FDF [21].
             * FDF must be set for any FD frame.
             * BRS enables the data-phase bit-rate switch.
             */
            elem[ 1U ] = ( ( uint32_t )frame->dlc & 0xFU ) << TXBE_T1_DLC_Pos;
            if( frame->fd )
            {
                elem[ 1U ] |= ( 1U << TXBE_T1_FDF_Pos );
            }
            if( frame->brs && frame->fd )
            {
                elem[ 1U ] |= ( 1U << TXBE_T1_BRS_Pos );
            }

            /*
             * Data words: pack bytes into 32-bit words, little-endian.
             * Write all CAN_FD_DATA_WORDS words regardless of actual payload
             * size — unused words are irrelevant (DLC controls how many bytes
             * the receiver reads) but zeroing them avoids stale RAM data.
             */
            uint32_t byte_count = dlc_to_bytes( frame->dlc );

            for( uint32_t w = 0U; w < CAN_FD_DATA_WORDS; ++w )
            {
                uint32_t b = w * 4U;
                elem[ CAN_FD_HEADER_WORDS + w ] =
                    ( ( b + 0U < byte_count ) ? ( ( uint32_t )frame->data[ b + 0U ]        ) : 0U ) |
                    ( ( b + 1U < byte_count ) ? ( ( uint32_t )frame->data[ b + 1U ] <<  8U ) : 0U ) |
                    ( ( b + 2U < byte_count ) ? ( ( uint32_t )frame->data[ b + 2U ] << 16U ) : 0U ) |
                    ( ( b + 3U < byte_count ) ? ( ( uint32_t )frame->data[ b + 3U ] << 24U ) : 0U );
            }

            p_can->TXBAR = ( 1U << put_idx );
            result = RES_OK;
        }
    }

    return result;
}

result_t can_recv( FDCAN_GlobalTypeDef * p_can,
                   can_frame_t * frame,
                   uint32_t timeout_ms )
{
    result_t res = RES_OK;

    if( ( p_can == NULL ) || ( frame == NULL ) )
    {
        res = RES_ERR_INVALID_ARG;
    }
    else if( ( p_can->PSR & FDCAN_PSR_BO ) != 0U )
    {
        res = RES_ERR_BUS;
    }
    else
    {
        uint32_t start = delay_get_tick();

        while( ( ( p_can->RXF0S & FDCAN_RXF0S_F0FL_Msk ) == 0U ) &&
               ( !timed_out( start, timeout_ms ) ) )
        {
            /* Wait until at least one element is in RX FIFO 0. */
        }

        if( ( p_can->RXF0S & FDCAN_RXF0S_F0FL_Msk ) == 0U )
        {
            res = RES_ERR_TIMEOUT;
        }

        if( RES_IS_OK( res ) )
        {
            uint32_t get_idx =
                ( p_can->RXF0S & FDCAN_RXF0S_F0GI_Msk ) >> FDCAN_RXF0S_F0GI_Pos;

            uint32_t   base = ram_base( p_can );
            uint32_t * elem = ( uint32_t * )
                ( base + RAM_OFF_RXF0 + ( get_idx * CAN_ELEMENT_SIZE_BYTES ) );

            /* R0: standard ID */
            frame->id = ( elem[ 0U ] >> 18U ) & STD_FILTER_ID_MASK;

            /* R1: DLC, FDF, BRS */
            frame->dlc = ( uint8_t )
                ( ( elem[ 1U ] >> RXBE_R1_DLC_Pos ) & 0xFU );
            frame->fd  = ( ( elem[ 1U ] >> RXBE_R1_FDF_Pos ) & 1U ) != 0U;
            frame->brs = ( ( elem[ 1U ] >> RXBE_R1_BRS_Pos ) & 1U ) != 0U;

            /* Data words: unpack all up to actual payload length */
            uint32_t byte_count = dlc_to_bytes( frame->dlc );

            for( uint32_t w = 0U; w < CAN_FD_DATA_WORDS; ++w )
            {
                uint32_t word = elem[ CAN_FD_HEADER_WORDS + w ];
                uint32_t b    = w * 4U;

                if( b + 0U < byte_count ) { frame->data[ b + 0U ] = ( uint8_t )( word        ); }
                if( b + 1U < byte_count ) { frame->data[ b + 1U ] = ( uint8_t )( word >>  8U ); }
                if( b + 2U < byte_count ) { frame->data[ b + 2U ] = ( uint8_t )( word >> 16U ); }
                if( b + 3U < byte_count ) { frame->data[ b + 3U ] = ( uint8_t )( word >> 24U ); }
            }

            p_can->RXF0A = get_idx;
        }
    }

    return res;
}
