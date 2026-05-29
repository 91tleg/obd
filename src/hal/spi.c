/**
 * @file    spi.c
 * @brief   STM32H753 SPI HAL implementation.
 */

#include "hal/spi.h"

static uint32_t mode_to_cfg2( spi_mode_t mode )
{
    uint32_t cfg2 = 0U;

    switch( mode )
    {
        case SPI_MODE_0:
            /* CPOL=0 CPHA=0 — both bits clear */
            break;
        case SPI_MODE_1:
            cfg2 |= SPI_CFG2_CPHA;
            break;
        case SPI_MODE_2:
            cfg2 |= SPI_CFG2_CPOL;
            break;
        case SPI_MODE_3:
            cfg2 |= ( SPI_CFG2_CPOL | SPI_CFG2_CPHA );
            break;
        default:
            break;
    }

    return cfg2;
}

result_t spi_init( SPI_TypeDef * p_spi,
                   spi_mode_t mode,
                   spi_div_t div,
                   spi_firstbit_t firstbit )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_spi != NULL )
    {
        p_spi->CR1 &= ~SPI_CR1_SPE;

        /*
         * CFG1:
         *   MBR     [5:3]  baud rate divider
         *   DSIZE   [4:0]  data size — 0x07 = 8-bit
         *   FTHLV   [8:5]  FIFO threshold — 0 = 1 data frame
         */
        p_spi->CFG1 = ( ( ( uint32_t )div  << SPI_CFG1_MBR_Pos   ) & SPI_CFG1_MBR_Msk   ) |
                      ( ( 7U               << SPI_CFG1_DSIZE_Pos ) & SPI_CFG1_DSIZE_Msk ) |
                      ( ( 0U               << SPI_CFG1_FTHLV_Pos ) & SPI_CFG1_FTHLV_Msk );

        /*
         * CFG2:
         *   CPOL / CPHA  from mode
         *   LSBFRST      bit order
         *   MASTER       master mode
         *   SSM          software NSS management
         *   SSOE         0 — NSS driven by GPIO not SPI
         *   COMM [18:17] 01 = simplex transmitter (TX only)
         */
        p_spi->CFG2 = mode_to_cfg2( mode )                                         |
                      ( ( firstbit == SPI_FIRSTBIT_LSB ) ? SPI_CFG2_LSBFRST : 0U ) |
                      SPI_CFG2_MASTER                                              |
                      SPI_CFG2_SSM                                                 |
                      ( 0x1U << SPI_CFG2_COMM_Pos );   /* simplex TX           */

        /*
         * CR2: TSIZE = 0 — reload mode, transfer runs until SPE cleared.
         * This avoids having to set TSIZE for each variable-length transfer.
         */
        p_spi->CR2 = 0U;

        /* SSOE must be 0 when SSM=1 on H753 */
        p_spi->CR1 &= ~SPI_CR1_SSI;
        p_spi->CR1 |=  SPI_CR1_SSI;   /* internal NSS high while SSM=1 */

        /* enable */
        p_spi->CR1 |= SPI_CR1_SPE;

        /* start — required on H7 to enable TX path */
        p_spi->CR1 |= SPI_CR1_CSTART;

        result = RES_OK;
    }

    return result;
}

result_t spi_write_byte( SPI_TypeDef * p_spi, uint8_t data )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_spi != NULL )
    {
        while( ( p_spi->SR & SPI_SR_TXP ) == 0U )
        {
            /* wait for TX FIFO to have space */
        }

        /* write one byte — 8-bit access to TXDR */
        *( ( volatile uint8_t * )&p_spi->TXDR ) = data;

        result = RES_OK;
    }

    return result;
}

result_t spi_write_buf( SPI_TypeDef * p_spi,
                        uint8_t const * buf,
                        uint32_t len )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( ( p_spi != NULL ) && ( buf != NULL ) && ( len > 0U ) )
    {
        uint32_t i = 0U;

        while( i < len )
        {
            while( ( p_spi->SR & SPI_SR_TXP ) == 0U )
            {
                /* wait for TX FIFO space */
            }

            *( ( volatile uint8_t * )&p_spi->TXDR ) = buf[ i ];
            ++i;
        }

        result = RES_OK;
    }

    return result;
}

result_t spi_wait_idle( SPI_TypeDef const * p_spi )
{
    result_t result = RES_ERR_INVALID_ARG;

    if( p_spi != NULL )
    {
        while( ( p_spi->SR & SPI_SR_TXC ) == 0U )
        {
            /* wait for TX FIFO empty */
        }

        result = RES_OK;
    }

    return result;
}
