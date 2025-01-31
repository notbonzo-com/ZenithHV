//
// Created by notbonzo on 1/30/25.
//

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>

void outb( uint16_t port, uint8_t value );
void outw( uint16_t port, uint16_t value );
void outl( uint16_t port, uint32_t value );

uint8_t inb( uint16_t port );
uint16_t inw( uint16_t port );
uint32_t inl( uint16_t port );

void cli( );
void sti( );
void hlt( );

bool is_interrupts_enabled( );

uint64_t read_msr( uint32_t msr );
void write_msr( uint32_t msr, uint64_t value );

void io_wait( );

void pause( );

#endif //IO_H
