/**
 * @file  result.h
 * @brief Global status and error codes.
 */

#ifndef LIB_CORE_RESULT_H
#define LIB_CORE_RESULT_H

/**
 * @brief Standardized result codes for function returns.
 */
typedef enum
{
    RES_OK              = 0U,   /* success                               */
    RES_ERR_INVALID_ARG = 1U,   /* null pointer or bad parameter         */
    RES_ERR_TIMEOUT     = 2U,   /* operation timed out                   */
    RES_ERR_BUS         = 3U,   /* I2C/SPI/UART bus error                */
    RES_ERR_CHECKSUM    = 4U,   /* data integrity failure                */
    RES_ERR_NO_DATA     = 5U,   /* buffer empty, nothing to read         */
    RES_ERR_OVERFLOW    = 6U,   /* buffer full, data lost                */
    RES_ERR_NOT_READY   = 7U,   /* peripheral not initialized            */
    RES_ERR_NOT_FOUND   = 8U,   /* device did not respond                */
    RES_ERR_UNSUPPORTED = 9U,   /* operation not supported               */
    RES_ERR_INTERNAL    = 10U,  /* should never happen                   */
} result_t;

#define RES_OK( r )     ( ( r ) == RES_OK )
#define RES_FAILED( r ) ( ( r ) != RES_OK )

#endif /* LIB_CORE_RESULT_H */
