//
// Created by notbonzo on 1/30/25.
//

#include <core/printf.h>
#include <dev/tty.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

static int tty_putc( char c, void* ctx )
{
    (void)ctx;
    tty.putc( c );
    return 0;
}

static int buffer_putc( char c, void* ctx )
{
    if( !ctx )
        return -1;

    char** buf_ptr = (char **)ctx;
    **buf_ptr = c;
    (*buf_ptr)++;
    return 0;
}

static int print_str( putc_func_t putc_func, void* ctx, const char* str )
{
    int count = 0;
    if( str )
    {
        while ( *str )
        {
            if( putc_func(*str, ctx) != 0 )
                return -1;
            count++;
            str++;
        }
    }
    return count;
}

static char* ull_to_base( unsigned long long value, char* buffer, int base, int uppercase )
{
    static const char* digits_lc = "0123456789abcdef";
    static const char* digits_uc = "0123456789ABCDEF";
    const char* digits = uppercase ? digits_uc : digits_lc;

    char temp[65];
    int pos = 0;

    if( base < 2 || base > 16 )
    {
        base = 10;
    }

    if( value == 0 )
    {
        temp[pos++] = '0';
    }
    else
    {
        while( value > 0 )
        {
            temp[pos++] = digits[value % base];
            value /= base;
        }
    }

    int i = 0;
    while( pos > 0 )
    {
        buffer[i++] = temp[--pos];
    }
    buffer[i] = '\0';
    return buffer;
}

static char* ll_to_base( long long value, char* buffer, int base, int uppercase )
{
    if( value < 0 )
    {
        unsigned long long tmp = (unsigned long long)(-value);
        ull_to_base( tmp, buffer+1, base, uppercase );
        buffer[0] = '-';
    }
    else
    {
        ull_to_base( (unsigned long long)value, buffer, base, uppercase );
    }
    return buffer;
}

static int print_number( putc_func_t putc_func, void* ctx,
                         int sign, long long value,
                         unsigned long long uvalue,
                         int base, int width, int zero_pad,
                         int uppercase, int left_justify )
{
    char buffer[68];
    if( sign )
        ll_to_base( value, buffer, base, uppercase );
    else
        ull_to_base( uvalue, buffer, base, uppercase );

    int len = (int)strlen( buffer );
    int total = 0;

    if( left_justify )
    {
        int ret = print_str( putc_func, ctx, buffer );
        if( ret < 0 ) return -1;
        total += ret;

        int pad = (width > len) ? (width - len) : 0;
        while( pad-- > 0 )
        {
            if( putc_func(' ', ctx) != 0 ) return -1;
            total++;
        }
    }
    else
    {
        if( width > 0 && len < width )
        {
            int pad = width - len;
            char pad_char = zero_pad ? '0' : ' ';

            if( zero_pad && buffer[0] == '-' )
            {
                if( putc_func('-', ctx) != 0 ) return -1;
                total++;
                memmove(buffer, buffer + 1, len);
                len--;
            }

            for( int i = 0; i < pad; i++ )
            {
                if( putc_func(pad_char, ctx) != 0 ) return -1;
                total++;
            }
        }

        int ret = print_str( putc_func, ctx, buffer );
        if( ret < 0 ) return -1;
        total += ret;
    }

    return total;
}


int fprintf( putc_func_t putc_func, void* ctx, const char* format, va_list args )
{
    if( !putc_func || !format )
        return -1;

    int total_count = 0;

    while( *format )
    {
        if( *format != '%' )
        {
            if( putc_func(*format, ctx) != 0 )
                return -1;
            total_count++;
            format++;
            continue;
        }

        format++;

        if( *format == '%' )
        {
            if( putc_func('%', ctx) != 0 )
                return -1;
            total_count++;
            format++;
            continue;
        }

        int zero_pad = 0;
        int width = 0;
        int length_mod = 0;
        int left_justify = 0;

        while( *format == '0' || *format == '-' )
        {
            if (*format == '0' && !left_justify)
                zero_pad = 1;
            if (*format == '-')
                left_justify = 1;
            format++;
        }

        if( *format >= '1' && *format <= '9' )
        {
            width = 0;
            while( *format >= '0' && *format <= '9' )
            {
                width = width * 10 + (*format - '0');
                format++;
            }
        }

        if( *format == 'h' )
        {
            length_mod = 1;
            format++;
        }
        else if( *format == 'l' )
        {
            length_mod = 2;
            format++;
            if( *format == 'l' )
            {
                length_mod = 3;
                format++;
            }
        }

        char spec = *format;
        if( spec == '\0' )
            break;

        format++;

        if( spec == 'c' )
        {
            char c = (char) va_arg( args, int );
            if( putc_func( c, ctx ) != 0 )
                return -1;
            total_count++;
        }
        else if( spec == 's' )
        {
            const char* s = va_arg( args, const char* );
            if( !s )
                s = "(null)";

            int len = (int)strlen( s );
            int pad = (width > len) ? (width - len) : 0;

            if( !left_justify )
            {
                while( pad-- > 0 )
                {
                    if( putc_func(' ', ctx) != 0 ) return -1;
                    total_count++;
                }
            }

            int ret = print_str( putc_func, ctx, s );
            if( ret < 0 ) return -1;
            total_count += ret;

            if( left_justify )
            {
                while( pad-- > 0 )
                {
                    if( putc_func(' ', ctx) != 0 ) return -1;
                    total_count++;
                }
            }
        }
        else if( spec == 'd' || spec == 'i' )
        {
            // signed integer
            long long val = 0;
            switch( length_mod )
            {
                case 1: // short
                    val = (short) va_arg( args, int );
                    break;
                case 2: // long
                    val = (long) va_arg( args, long );
                    break;
                case 3: // long long
                    val = (long long) va_arg( args, long long );
                    break;
                default:
                    val = (int) va_arg( args, int );
                    break;
            }

            int ret = print_number( putc_func, ctx,
                                    /*sign*/1, val, 0ULL,
                                    10, width, zero_pad, 0, left_justify );
            if( ret < 0 )
                return -1;
            total_count += ret;
        }
        else if( spec == 'u' )
        {
            // unsigned integer
            unsigned long long val = 0ULL;
            switch( length_mod )
            {
                case 1: // short
                    val = (unsigned short) va_arg( args, unsigned int );
                    break;
                case 2: // long
                    val = (unsigned long) va_arg( args, unsigned long );
                    break;
                case 3: // long long
                    val = (unsigned long long) va_arg( args, unsigned long long );
                    break;
                default:
                    val = (unsigned int) va_arg( args, unsigned int );
                    break;
            }

            int ret = print_number( putc_func, ctx,
                                    /*sign*/0, 0LL, val,
                                    10, width, zero_pad, 0, left_justify );
            if( ret < 0 )
                return -1;
            total_count += ret;
        }
        else if( spec == 'x' || spec == 'X' )
        {
            // hex integer
            unsigned long long val = 0ULL;
            switch( length_mod )
            {
                case 1: // short
                    val = (unsigned short) va_arg( args, unsigned int );
                    break;
                case 2: // long
                    val = (unsigned long) va_arg( args, unsigned long );
                    break;
                case 3: // long long
                    val = (unsigned long long) va_arg( args, unsigned long long );
                    break;
                default:
                    val = (unsigned int) va_arg( args, unsigned int );
                    break;
            }

            int ret = print_number( putc_func, ctx,
                                    /*sign*/0, 0LL, val,
                                    16, width, zero_pad, (spec == 'X'), left_justify );
            if( ret < 0 )
                return -1;
            total_count += ret;
        }
        else if( spec == 'p' )
        {
            uintptr_t ptr = ( uintptr_t ) va_arg( args, void* );
            char buffer[20];

            ull_to_base( ptr, buffer, 16, 0 );

            int ret = print_str( putc_func, ctx, buffer );
            if( ret < 0 ) return -1;
            total_count += ret;
        }
        else
        {
            // unknown specifier, print literally as `%<spec>`
            if( putc_func('%', ctx) != 0 )
                return -1;
            if( putc_func(spec, ctx) != 0 )
                return -1;
            total_count += 2;
        }
    }
    return total_count;
}

int printf( const char* format, ... )
{
    va_list args;
    va_start( args, format );
    int ret = vprintf( format, args );
    va_end( args );
    return ret;
}

int vprintf( const char* format, va_list args )
{
    return fprintf( tty_putc, nullptr, format, args );
}

int sprintf( char* buffer, const char* format, ... )
{
    va_list args;
    va_start( args, format );
    int ret = vsprintf( buffer, format, args );
    va_end( args );
    return ret;
}

int vsprintf( char* buffer, const char* format, va_list args )
{
    if( !buffer )
        return -1;

    int ret = fprintf( buffer_putc, (void*)&buffer, format, args );
    if( ret >= 0 )
    {
        *buffer = '\0';
    }
    return ret;
}
