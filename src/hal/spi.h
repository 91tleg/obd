/**
 * @file    spi.h
 * @brief   STM32H753 SPI HAL
 *
 *          write-only master mode.
 */

#ifndef HAL_SPI_H
#define HAL_SPI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cmsis/stm32h753xx.h"
#include "lib/core/result.h"

typedef enum
{
    SPI_MODE_0 = 0U,   /* CPOL=0 CPHA=0 — clock idle low,  sample rising  */
    SPI_MODE_1 = 1U,   /* CPOL=0 CPHA=1 — clock idle low,  sample falling */
    SPI_MODE_2 = 2U,   /* CPOL=1 CPHA=0 — clock idle high, sample falling */
    SPI_MODE_3 = 3U,   /* CPOL=1 CPHA=1 — clock idle high, sample rising  */
} spi_mode_t;

typedef enum
{
    SPI_DIV_2   = 0U,
    SPI_DIV_4   = 1U,
    SPI_DIV_8   = 2U,
    SPI_DIV_16  = 3U,
    SPI_DIV_32  = 4U,
    SPI_DIV_64  = 5U,
    SPI_DIV_128 = 6U,
    SPI_DIV_256 = 7U,
} spi_div_t;

typedef enum
{
    SPI_FIRSTBIT_MSB = 0U,
    SPI_FIRSTBIT_LSB = 1U,
} spi_firstbit_t;

/**
 * Initialize SPI peripheral as master, 8-bit, TX-only.
 *
 * @param p_spi     SPI1..SPI6 peripheral pointer.
 * @param mode      Clock polarity and phase.
 * @param div       Clock prescaler — fPCLK / div.
 * @param firstbit  MSB or LSB first.
 * @return RES_OK or RES_ERR_INVALID_ARG.
 */
result_t spi_init( SPI_TypeDef * p_spi,
                   spi_mode_t mode,
                   spi_div_t div,
                   spi_firstbit_t firstbit );

/**
 * Transmit one byte — blocking until TX FIFO accepts the byte.
 * Does not wait for transfer complete — caller manages CS timing.
 *
 * @return RES_OK or RES_ERR_INVALID_ARG.
 */
result_t spi_write_byte( SPI_TypeDef * p_spi, uint8_t data );

/**
 * Transmit a buffer of bytes — blocking until all bytes sent.
 *
 * @return RES_OK or RES_ERR_INVALID_ARG.
 */
result_t spi_write_buf( SPI_TypeDef *  p_spi,
                        uint8_t const * buf,
                        uint32_t len );

/**
 * Wait for the SPI peripheral to finish all pending transfers.
 * Call before deasserting CS after a burst write.
 *
 * @return RES_OK or RES_ERR_INVALID_ARG.
 */
result_t spi_wait_idle( SPI_TypeDef const * p_spi );

static inline bool spi_flag_txp( SPI_TypeDef const * p_spi )
{
    bool result = false;

    if( p_spi != NULL )
    {
        result = ( ( p_spi->SR & SPI_SR_TXP ) != 0U );
    }

    return result;
}

static inline bool spi_flag_rxp( SPI_TypeDef const * p_spi )
{
    bool result = false;

    if( p_spi != NULL )
    {
        result = ( ( p_spi->SR & SPI_SR_RXP ) != 0U );
    }

    return result;
}

static inline bool spi_flag_txc( SPI_TypeDef const * p_spi )
{
    bool result = false;

    if( p_spi != NULL )
    {
        result = ( ( p_spi->SR & SPI_SR_TXC ) != 0U );
    }

    return result;
}

static inline bool spi_flag_ovr( SPI_TypeDef const * p_spi )
{
    bool result = false;

    if( p_spi != NULL )
    {
        result = ( ( p_spi->SR & SPI_SR_OVR ) != 0U );
    }

    return result;
}

static inline void spi_clear_ovr( SPI_TypeDef * p_spi )
{
    if( p_spi != NULL )
    {
        p_spi->IFCR = SPI_IFCR_OVRC;
    }
}

static inline void spi_enable( SPI_TypeDef * p_spi )
{
    if( p_spi != NULL )
    {
        p_spi->CR1 |= SPI_CR1_SPE;
    }
}

static inline void spi_disable( SPI_TypeDef * p_spi )
{
    if( p_spi != NULL )
    {
        p_spi->CR1 &= ~SPI_CR1_SPE;
    }
}

/**
 * Start a transfer — required on H753 after SPE set.
 * Sets CSTART in CR1 to begin the transaction.
 */
static inline void spi_start( SPI_TypeDef * p_spi )
{
    if( p_spi != NULL )
    {
        p_spi->CR1 |= SPI_CR1_CSTART;
    }
}

#endif /* HAL_SPI_H */
