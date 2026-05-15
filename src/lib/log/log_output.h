/**
 * @file    log_output.h
 * @brief   Log output contract.
 */

#ifndef LIB_LOG_LOG_OUTPUT_H
#define LIB_LOG_LOG_OUTPUT_H

#include <stdint.h>

/**
 * Emit a formatted log line.
 *
 * @param buf   Null-terminated string to emit. Not guaranteed to remain
 *              valid after this call returns — do not store the pointer.
 * @param len   Number of bytes to write (excludes null terminator).
 */
void log_write_output( const char * buf, uint32_t len );

#endif /* LIB_LOG_LOG_OUTPUT_H */
