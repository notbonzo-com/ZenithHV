#include "main.h"
#include <stdint.h>
#include <limine.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <util/binary.h>

void VISUAL_SetPixel(uint32_t x, uint32_t y, uint32_t color, uint8_t value) {
    if (x >= g_Framebuffer->width || y >= g_Framebuffer->height)
        return; // Out of bounds check

    // Calculate the position in the framebuffer's memory
    uint32_t fb_pos = (y * g_Framebuffer->pitch) + (x * (g_Framebuffer->bpp / 8));

    // Set the pixel to the specified color or 0
    uint32_t *pixel = (uint32_t *)((uintptr_t)g_Framebuffer->address + fb_pos);
    *pixel = value ? color : 0;
}


void VISUAL_DrawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        VISUAL_SetPixel(x0, y0, color, 1);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void VISUAL_DrawCircleHollow(int cx, int cy, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    while (x >= y) {
        VISUAL_SetPixel(cx + x, cy + y, color, 1);
        VISUAL_SetPixel(cx - x, cy + y, color, 1);
        VISUAL_SetPixel(cx + x, cy - y, color, 1);
        VISUAL_SetPixel(cx - x, cy - y, color, 1);
        VISUAL_SetPixel(cx + y, cy + x, color, 1);
        VISUAL_SetPixel(cx - y, cy + x, color, 1);
        VISUAL_SetPixel(cx + y, cy - x, color, 1);
        VISUAL_SetPixel(cx - y, cy - x, color, 1);

        y++;

        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }
    }
}

void VISUAL_DrawCircle(int cx, int cy, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    while (x >= y) {
        // Draw horizontal lines for each pair of points
        VISUAL_DrawLine(cx - x, cy + y, cx + x, cy + y, color); // Top
        VISUAL_DrawLine(cx - y, cy + x, cx + y, cy + x, color); // Right
        VISUAL_DrawLine(cx - x, cy - y, cx + x, cy - y, color); // Bottom
        VISUAL_DrawLine(cx - y, cy - x, cx + y, cy - x, color); // Left

        y++;

        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }
    }
}

void VISUAL_DrawSquare(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            // Draw the square
            VISUAL_SetPixel(x + j, y + i, color, 1);
        }
    }
}


#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}

void VISUAL_DrawHeaderImage(char *data, uint32_t width, uint32_t height, uint32_t startX, uint32_t startY) {
    unsigned char pixel[3]; // Array to hold the decoded pixel (RGB)

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            // Decode the pixel data
            HEADER_PIXEL(data, pixel);

            // Combine the RGB values into a single 32-bit integer
            // Assuming the format is 0x00RRGGBB
            uint32_t color = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];

            // Draw the pixel on the framebuffer
            VISUAL_SetPixel(startX + x, startY + y, color, 1);
        }
    }
}

void VISUAL_ClrSrc()
{
    VISUAL_DrawSquare(0, 0, 1024, 2024, 0x000000);
    VISUAL_DrawSquare(1024, 0, 1024, 2024, 0x000000);
}