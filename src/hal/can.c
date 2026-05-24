/**
 * @file    can.c
 * @brief   STM32H753 FDCAN HAL implementation.
 *
 * Message RAM layout (per peripheral, offsets from peripheral RAM base):
 *
 *   0x000  Standard ID filter list  (1 element  × 4 B  =   4 B)
 *   0x004  RX FIFO 0                (8 elements × 16 B = 128 B)
 *   0x084  TX FIFO                  (8 elements × 16 B = 128 B)
 */

#include "can.h"
#include <stddef.h>
#include "lib/time/delay_tick.h"

#define FDCAN1_RAM_BASE  ( SRAMCAN_BASE )
#define FDCAN2_RAM_BASE  ( SRAMCAN_BASE + 0x0550U )

#define NUM_STD_FILTERS        ( 1U )
#define NUM_RX_FIFO0_ELEMENTS  ( 8U )
#define NUM_TX_FIFO_ELEMENTS   ( 8U )

/* Each classic-CAN element is 2 header words + 2 data words = 16 bytes. */
#define CAN_ELEMENT_SIZE_BYTES ( 16U )

#define RAM_OFF_STD_FILTER  ( 0x000U )
#define RAM_OFF_RXF0        ( RAM_OFF_STD_FILTER + ( NUM_STD_FILTERS * 4U ) )
#define RAM_OFF_TXF         ( RAM_OFF_RXF0 + ( NUM_RX_FIFO0_ELEMENTS * CAN_ELEMENT_SIZE_BYTES ) )

#define STD_FILTER_SFEC_RXF0   ( 0x3U )   /**< Store matching frames in RX FIFO 0. */
#define STD_FILTER_SFT_RANGE   ( 0x1U )   /**< Range filter: accept SFID2..SFID1.  */

#define STD_FILTER_SFEC_Pos    ( 30U )
#define STD_FILTER_SFT_Pos     ( 27U )
#define STD_FILTER_SFID1_Pos   ( 16U )
#define STD_FILTER_SFID2_Pos   (  0U )
#define STD_FILTER_ID_MASK     ( 0x7FFU )

#define TXBE_T1_DLC_Pos        ( 16U )

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
                   can_filter_t const * filter )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_can != NULL ) && ( filter != NULL ) )
    {
        uint32_t   start = 0U;
        uint32_t   base  = 0U;
        uint32_t * ram   = NULL;

        /* Request init mode. */
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
            /* Enable configuration change. */
            p_can->CCCR |= FDCAN_CCCR_CCE;

            p_can->NBTP = timing;

            /*
             * SIDFC: standard filter list start address and element count.
             * FLSSA is the word address shifted to the correct bit position;
             * the offset is in bytes so we shift right by 2 to get words.
             */
            p_can->SIDFC =
                ( ( ( RAM_OFF_STD_FILTER >> 2U ) << FDCAN_SIDFC_FLSSA_Pos ) &
                    FDCAN_SIDFC_FLSSA_Msk ) |
                ( NUM_STD_FILTERS << FDCAN_SIDFC_LSS_Pos );

            /*
             * RXF0C: RX FIFO 0 start address and element count.
             * F0SA is the word address of the FIFO start.
             */
            p_can->RXF0C =
                ( ( ( RAM_OFF_RXF0 >> 2U ) << FDCAN_RXF0C_F0SA_Pos ) &
                    FDCAN_RXF0C_F0SA_Msk ) |
                ( NUM_RX_FIFO0_ELEMENTS << FDCAN_RXF0C_F0S_Pos );

            /*
             * TXBC: TX buffer start address and FIFO element count.
             * TBSA is the word address of the TX buffer start.
             */
            p_can->TXBC =
                ( ( ( RAM_OFF_TXF >> 2U ) << FDCAN_TXBC_TBSA_Pos ) &
                    FDCAN_TXBC_TBSA_Msk ) |
                ( NUM_TX_FIFO_ELEMENTS << FDCAN_TXBC_TFQS_Pos );

            /*
             * RXESC / TXESC: 0 = 8-byte data field (default after reset).
             * Written explicitly so the intent is clear and reset-value
             * reliance is not silently assumed.
             */
            p_can->RXESC = 0U;
            p_can->TXESC = 0U;

            base = ram_base( p_can );
            ram  = ( uint32_t * )( base + RAM_OFF_STD_FILTER );

            /*
             * Single range filter element (word 0 only for std-ID filters):
             *   SFEC = 0x3   store matching frames in RX FIFO 0
             *   SFT  = 0x1   range filter (SFID2 ≤ ID ≤ SFID1)
             *   SFID1        upper bound (id_high)
             *   SFID2        lower bound (id_low)
             */
            *ram =
                ( ( STD_FILTER_SFEC_RXF0                           ) << STD_FILTER_SFEC_Pos  ) |
                ( ( STD_FILTER_SFT_RANGE                           ) << STD_FILTER_SFT_Pos   ) |
                ( ( filter->id_high & STD_FILTER_ID_MASK           ) << STD_FILTER_SFID1_Pos ) |
                ( ( filter->id_low  & STD_FILTER_ID_MASK           ) << STD_FILTER_SFID2_Pos );

            /* Reject all non-matching standard and extended frames. */
            p_can->GFC = ( FDCAN_GFC_ANFE_Msk | FDCAN_GFC_ANFS_Msk );

            /* Leave init mode */
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

            /*
             * T0: standard ID in bits [28:18], XTD=0 (std), RTR=0 (data frame).
             * The ID occupies the same position as the RX R0 field so the
             * same >> 18 extraction works on receive.
             */
            elem[ 0 ] = ( frame->id & STD_FILTER_ID_MASK ) << 18U;

            /*
             * T1: DLC in bits [19:16].
             * frame->dlc is the classical DLC (0–8), identical to the
             * FDCAN DLC encoding for payloads ≤ 8 bytes.
             */
            elem[ 1 ] = ( ( uint32_t )frame->dlc & 0xFU ) << TXBE_T1_DLC_Pos;

            /* T2, T3: data bytes, little-endian within each word. */
            elem[ 2 ] = ( ( uint32_t )frame->data[ 0 ]        ) |
                        ( ( uint32_t )frame->data[ 1 ] <<  8U ) |
                        ( ( uint32_t )frame->data[ 2 ] << 16U ) |
                        ( ( uint32_t )frame->data[ 3 ] << 24U );
            elem[ 3 ] = ( ( uint32_t )frame->data[ 4 ]        ) |
                        ( ( uint32_t )frame->data[ 5 ] <<  8U ) |
                        ( ( uint32_t )frame->data[ 6 ] << 16U ) |
                        ( ( uint32_t )frame->data[ 7 ] << 24U );

            /* Request transmission of the filled element. */
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

            /*
             * R0: standard ID in bits [28:18].
             */
            frame->id = ( elem[ 0 ] >> 18U ) & STD_FILTER_ID_MASK;

            /*
             * R1: DLC in bits [19:16].
             * For classic CAN (RXESC=0) values 0–8 map directly to byte count.
             */
            frame->dlc = ( uint8_t )( ( elem[ 1 ] >> TXBE_T1_DLC_Pos ) & 0xFU );

            /* R2, R3: data bytes. */
            frame->data[ 0 ] = ( uint8_t )( elem[ 2 ]        );
            frame->data[ 1 ] = ( uint8_t )( elem[ 2 ] >>  8U );
            frame->data[ 2 ] = ( uint8_t )( elem[ 2 ] >> 16U );
            frame->data[ 3 ] = ( uint8_t )( elem[ 2 ] >> 24U );
            frame->data[ 4 ] = ( uint8_t )( elem[ 3 ]        );
            frame->data[ 5 ] = ( uint8_t )( elem[ 3 ] >>  8U );
            frame->data[ 6 ] = ( uint8_t )( elem[ 3 ] >> 16U );
            frame->data[ 7 ] = ( uint8_t )( elem[ 3 ] >> 24U );

            /* Acknowledge: release the element back to hardware. */
            p_can->RXF0A = get_idx;
        }
    }

    return res;
}
