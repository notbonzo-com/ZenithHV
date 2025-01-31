//
// Created by notbonzo on 1/30/25.
//

#ifndef TTY_H
#define TTY_H

#include <common/constants.h>
#include <stddef.h>

#define TTY_BUFFER_SIZE 1024

struct tty {
    void(*putc)( char c );
    char buffer[TTY_BUFFER_SIZE];
    size_t buffer_pos;
};

extern struct tty tty;

void init_tty( );

#endif //TTY_H
