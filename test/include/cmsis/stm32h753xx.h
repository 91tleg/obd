/**
 * @file    stm32h753xx.h
 * @brief   Minimal CMSIS shim.
 *
 * Replaces the real STM32 CMSIS header so can_tp.c and related
 * files compile without the ARM toolchain.
 *
 * Only the definitions actually referenced by the SIL build are included.
 * Register structs are present but their fields are never accessed at
 * runtime — can_stub.c intercepts all can_send/can_recv calls before
 * any register access occurs.
 */

#ifndef CMSIS_STM32H753XX_HOST_SHIM_H
#define CMSIS_STM32H753XX_HOST_SHIM_H

#include <stdint.h>

typedef struct
{
    uint32_t CCCR;
    uint32_t NBTP;
    uint32_t DBTP;
    uint32_t TEST;
    uint32_t RXF0C;
    uint32_t RXF0S;
    uint32_t RXF0A;
    uint32_t RXESC;
    uint32_t TXBC;
    uint32_t TXFQS;
    uint32_t TXBAR;
    uint32_t TXESC;
    uint32_t SIDFC;
    uint32_t GFC;
    uint32_t PSR;
} FDCAN_GlobalTypeDef;

#define FDCAN_CCCR_INIT         ( 1U <<  0U )
#define FDCAN_CCCR_CCE          ( 1U <<  1U )
#define FDCAN_CCCR_TEST         ( 1U <<  7U )
#define FDCAN_CCCR_FDOE         ( 1U <<  8U )
#define FDCAN_CCCR_BRSE         ( 1U <<  9U )

#define FDCAN_TEST_LBCK         ( 1U <<  4U )

#define FDCAN_PSR_BO            ( 1U <<  7U )

#define FDCAN_NBTP_NSJW_Pos     ( 25U )
#define FDCAN_NBTP_NBRP_Pos     ( 16U )
#define FDCAN_NBTP_NTSEG1_Pos   (  8U )
#define FDCAN_NBTP_NTSEG2_Pos   (  0U )

#define FDCAN_DBTP_DSJW_Pos     ( 28U )
#define FDCAN_DBTP_DBRP_Pos     ( 16U )
#define FDCAN_DBTP_DTSEG1_Pos   (  8U )
#define FDCAN_DBTP_DTSEG2_Pos   (  0U )

#define FDCAN_SIDFC_FLSSA_Pos   (  2U )
#define FDCAN_SIDFC_FLSSA_Msk   ( 0x3FFFU << FDCAN_SIDFC_FLSSA_Pos )
#define FDCAN_SIDFC_LSS_Pos     ( 16U )

#define FDCAN_RXF0C_F0SA_Pos    (  2U )
#define FDCAN_RXF0C_F0SA_Msk    ( 0x3FFFU << FDCAN_RXF0C_F0SA_Pos )
#define FDCAN_RXF0C_F0S_Pos     ( 16U )

#define FDCAN_RXF0S_F0FL_Msk    ( 0x7FU <<  0U )
#define FDCAN_RXF0S_F0GI_Msk    ( 0x3FU << 16U )
#define FDCAN_RXF0S_F0GI_Pos    ( 16U )

#define FDCAN_TXBC_TBSA_Pos     (  2U )
#define FDCAN_TXBC_TBSA_Msk     ( 0x3FFFU << FDCAN_TXBC_TBSA_Pos )
#define FDCAN_TXBC_TFQS_Pos     ( 24U )

#define FDCAN_TXFQS_TFQF        ( 1U << 21U )
#define FDCAN_TXFQS_TFQPI_Msk   ( 0x1FU << 16U )
#define FDCAN_TXFQS_TFQPI_Pos   ( 16U )

#define FDCAN_RXESC_F0DS_Pos    (  0U )
#define FDCAN_TXESC_TBDS_Pos    (  0U )

#define FDCAN_GFC_ANFE_Msk      ( 0x3U << 2U )
#define FDCAN_GFC_ANFS_Msk      ( 0x3U << 0U )

#define SRAMCAN_BASE            ( 0x4000AC00UL )

static FDCAN_GlobalTypeDef s_fdcan1_host;
static FDCAN_GlobalTypeDef s_fdcan2_host;

#define FDCAN1  ( &s_fdcan1_host )
#define FDCAN2  ( &s_fdcan2_host )

#endif /* CMSIS_STM32H753XX_HOST_SHIM_H */
