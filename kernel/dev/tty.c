//
// Created by notbonzo on 1/30/25.
//

#include <dev/tty.h>
#include <common/constants.h>
#include <string.h>

struct tty tty;

#if defined(DEBUG) && DEBUG_VM == 1
#include <arch/io.h>
void putc( char c ) {
    outb( 0xe9, c );
}
#endif

void init_tty( ) {
    memset( &tty, 0, sizeof( tty ) );
    tty.putc = putc;
}
