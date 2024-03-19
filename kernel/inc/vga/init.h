#pragma once

#include <limine.h>
#include <stdint.h>

typedef struct limine_framebuffer framebuffer_t;

extern framebuffer_t* g_Framebuffer;
void VISUAL_Init();
void VISUAL_ClrSrc();
void VISUAL_SetPixel(uint32_t x, uint32_t y, uint32_t color, uint8_t value);