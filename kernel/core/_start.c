#include <core/constants.h>
#include <arch/io.h>
#include <core/printf.h>

#include <dev/tty.h>

#include <arch/x86_64/gdt.h>
#include <arch/x86_64/intr.h>
#include <core/mm/pmm.h>

[[noreturn]] void _start( )
{
    init_tty( );
#ifdef DEBUG_PRINTF
    printf("--- Testing formatted printing ---\n");

    printf("Character: %c\n", 'A');
    printf("String: %s\n", "Test string");

    printf("Signed integer: %d\n", -123);
    printf("Unsigned integer: %u\n", 123);

    printf("Hex lowercase: %x\n", 0xabcdef);
    printf("Hex uppercase: %X\n", 0xABCDEF);

    printf("Padded integer (width 5): [%5d]\n", 42);
    printf("Padded integer (zero-pad, width 5): [%05d]\n", 42);
    printf("Left-padded string (width 10): [%10s]\n", "test");
    printf("Right-padded string (width -10): [%-10s]\n", "test");

    printf("Left-padded number (width 10): [%10d]\n", 76);
    printf("Right-padded number (width -10): [%-10d]\n", 67);

    printf("Zero: %d\n", 0);
    printf("Large number: %d\n", 2147483647);
    printf("Negative large number: %ld\n", -2147483648);
    printf("Unsigned large number: %lu\n", 42949672952U);

    printf("Long: %ld\n", 123456789L);
    printf("Long long: %lld\n", 9223372036854775807LL);
    printf("Unsigned long long: %llu\n", 18446744073709551615ULL);

    printf("Large hex number lowercase: %lx\n", 0xabcdef123456789);
    printf("Large hex number uppercase: %lX\n", 0xabcdef123456789);

    printf("Mixing types: char=%c, int=%d, hex=%x, string=%s\n", 'X', 255, 255, "mixing");

    printf("Literal percent: %%\n");
    printf("--- End of test ---\n");
#endif

    init_gdt( );
    init_idt( );
    init_pmm( );

#if defined(DEBUG) && DEBUG_VM < 3
    outw( VM_SHUTDOWN_PORT, VM_SHUTDOWN_MAGIC );
#endif

    cli( );
    for ( ;; ) {
        hlt( );
    }
}
