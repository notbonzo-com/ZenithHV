#pragma once
#include <stdint.h>

void VISUAL_SetPixel(uint32_t x, uint32_t y, uint32_t color, uint8_t value);
void VISUAL_DrawLine(int x0, int y0, int x1, int y1, uint32_t color);
void VISUAL_DrawCircleHollow(int cx, int cy, int radius, uint32_t color);
void VISUAL_DrawCircle(int cx, int cy, int radius, uint32_t color);
void VISUAL_DrawSquare(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
void VISUAL_DrawHeaderImage(char *data, uint32_t width, uint32_t height, uint32_t startX, uint32_t startY);
void VISUAL_ClrSrc();