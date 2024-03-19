#pragma once
#include <stdint.h>

void VISUAL_DrawCharacterASCII(int x, int y, uint32_t color, int asciiNumber);
void VISUAL_DrawCharacterSymbol(int x, int y, uint32_t color, unsigned char character);
void VISUAL_putc(const char c);
void VISUAL_puts(const char *s);