#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void io_wait(void);

static inline uint32_t mmioRead32(void *p)
{
    return *(volatile uint32_t *)(p);
}

static inline void mmioWrite32(void *p, uint32_t data)
{
    *(volatile uint32_t *)(p) = data;
}

static inline void mmioWrite8(void *p, uint8_t data)
{
    *(volatile uint8_t *)(p) = data;
}

static inline uint8_t mmioRead8(void *p)
{
    return *(volatile uint8_t *)(p);
}

static inline void mmioWrite16(void *p, uint16_t data)
{
    *(volatile uint16_t *)(p) = data;
}

static inline uint16_t mmioRead16(void *p)
{
    return *(volatile uint16_t *)(p);
}


static inline void mmioWrite64(void *p, uint64_t data)
{
    *(volatile uint64_t *)(p) = data;
}

static inline uint64_t mmioRead64(void *p)
{
    return *(volatile uint64_t *)(p);
}

#ifdef __cplusplus
}
#endif