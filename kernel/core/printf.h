//
// Created by notbonzo on 1/30/25.
//

#ifndef PRINTF_H
#define PRINTF_H

#include <stdarg.h>

/**
 * @brief A generic putc callback type.
 *
 * @param c The character to write.
 * @param ctx A user-defined context pointer (may be NULL).
 * @return 0 on success, or any error code you like.
 */
typedef int (*putc_func_t)(char c, void *ctx);

/**
 * @brief Core printing function that formats according to @p format
 *        and writes each character via the provided @p putc function.
 *
 * @param putc_func The callback used to print out each character.
 * @param ctx       A context pointer passed to @p putc_func (may be NULL).
 * @param format    The format string.
 * @param args      The va_list of arguments to format.
 * @return The number of characters printed or a negative value on error.
 */
int fprintf( putc_func_t putc_func, void *ctx, const char* format, va_list args );

/**
 * @brief A variant of fprintf() that accepts variadic arguments. Uses the global TTY.
 *
 * @param format The format string.
 * @param ...    Variadic arguments matching the format string.
 * @return The number of characters printed or a negative value on error.
 */
int printf( const char* format, ... );

/**
 * @brief A variant of fprintf() that uses a buffer for output.
 *
 * @param buffer Output buffer to store formatted text.
 * @param format The format string.
 * @param ...    Variadic arguments matching the format string.
 * @return The number of characters that would have been written,
 *         excluding the null-terminator.
 */
int sprintf( char* buffer, const char* format, ... );

/**
 * @brief A variant that takes a va_list, writing to a buffer.
 *
 * @param buffer Output buffer.
 * @param format Format string.
 * @param args   Already-initialized va_list.
 * @return The number of characters that would have been written,
 *         excluding the null-terminator.
 */
int vsprintf( char* buffer, const char* format, va_list args );

/**
 * @brief A variant of printf() that takes a va_list and prints via the global TTY.
 *
 * @param format Format string.
 * @param args   Already-initialized va_list.
 * @return Number of characters printed or a negative value on error.
 */
int vprintf( const char* format, va_list args );

#endif //PRINTF_H
