#include <stdint.h>
#include <stdbool.h>
#include <limine.h>

int abs(int value) {
    return (value < 0) ? -value : value;
}


struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};
void setPixel(struct limine_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t color, uint8_t value) {
    if (x >= framebuffer->width || y >= framebuffer->height)
        return; // Out of bounds check

    // Calculate the position in the framebuffer's memory
    uint32_t fb_pos = (y * framebuffer->pitch) + (x * (framebuffer->bpp / 8));

    // Set the pixel to the specified color or 0
    uint32_t *pixel = (uint32_t *)((uintptr_t)framebuffer->address + fb_pos);
    *pixel = value ? color : 0;
}


void draw_line(struct limine_framebuffer *framebuffer, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        setPixel(framebuffer, x0, y0, color, 1);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_circle(struct limine_framebuffer *framebuffer, int cx, int cy, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    while (x >= y) {
        setPixel(framebuffer, cx + x, cy + y, color, 1);
        setPixel(framebuffer, cx - x, cy + y, color, 1);
        setPixel(framebuffer, cx + x, cy - y, color, 1);
        setPixel(framebuffer, cx - x, cy - y, color, 1);
        setPixel(framebuffer, cx + y, cy + x, color, 1);
        setPixel(framebuffer, cx - y, cy + x, color, 1);
        setPixel(framebuffer, cx + y, cy - x, color, 1);
        setPixel(framebuffer, cx - y, cy - x, color, 1);

        y++;

        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }
    }
}

void draw_circle_full(struct limine_framebuffer *framebuffer, int cx, int cy, int radius, uint32_t color) {
    int x = radius;
    int y = 0;
    int p = 1 - radius;

    while (x >= y) {
        // Draw horizontal lines for each pair of points
        draw_line(framebuffer, cx - x, cy + y, cx + x, cy + y, color); // Top
        draw_line(framebuffer, cx - y, cy + x, cx + y, cy + x, color); // Right
        draw_line(framebuffer, cx - x, cy - y, cx + x, cy - y, color); // Bottom
        draw_line(framebuffer, cx - y, cy - x, cx + y, cy - x, color); // Left

        y++;

        if (p <= 0) {
            p = p + 2 * y + 1;
        } else {
            x--;
            p = p + 2 * y - 2 * x + 1;
        }
    }
}

void draw_square(struct limine_framebuffer *framebuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color) {
    for (uint32_t i = 0; i < height; i++) {
        for (uint32_t j = 0; j < width; j++) {
            // Draw the square
            setPixel(framebuffer, x + j, y + i, color, 1);
        }
    }
}

void draw_house(struct limine_framebuffer *framebuffer) {
        // Main structure of the house
    draw_square(framebuffer, 100, 150, 200, 200, 0xDAA520); // Goldenrod for the house

    // Roof
    draw_line(framebuffer, 100, 150, 200, 50, 0x8B4513); // SaddleBrown for the roof (left side)
    draw_line(framebuffer, 300, 150, 200, 50, 0x8B4513); // SaddleBrown for the roof (right side)

    // Windows
    draw_square(framebuffer, 120, 170, 40, 40, 0xFFFFFF); // White window (left)
    draw_line(framebuffer, 120, 190, 160, 190, 0x0000FF); // Horizontal crossbar (left window)
    draw_line(framebuffer, 140, 170, 140, 210, 0x0000FF); // Vertical crossbar (left window)

    draw_square(framebuffer, 240, 170, 40, 40, 0xFFFFFF); // White window (right)
    draw_line(framebuffer, 240, 190, 280, 190, 0x0000FF); // Horizontal crossbar (right window)
    draw_line(framebuffer, 260, 170, 260, 210, 0x0000FF); // Vertical crossbar (right window)

    // Door
    draw_square(framebuffer, 180, 270, 40, 80, 0x8B4513); // SaddleBrown for the door

    // Circular window on the roof
    draw_circle(framebuffer, 200, 100, 20, 0xFFFFFF); // White for the circular window
    draw_line(framebuffer, 200, 80, 200, 120, 0x0000FF); // Vertical crossbar (circular window)
    draw_line(framebuffer, 180, 100, 220, 100, 0x0000FF); // Horizontal crossbar (circular window)

    // Sun
    int sunX = 350, sunY = 50, sunRadius = 30;
    draw_circle_full(framebuffer, sunX, sunY, sunRadius, 0xFFFF00); // Yellow sun

    // Sunbeams
    int beamLength = 50; // Length of the sunbeams
    int approxDiagonalLength = beamLength * 7 / 10;  // Approximate 0.707 * beamLength

    // Gay
    // Define the end points of each sunbeam
    int beams[][2] = {
        {sunX - beamLength, sunY}, {sunX + beamLength, sunY},  // Horizontal left and right
        {sunX, sunY - beamLength}, {sunX, sunY + beamLength},  // Vertical top and bottom
        {sunX - approxDiagonalLength, sunY - approxDiagonalLength},  // Diagonal top-left
        {sunX + approxDiagonalLength, sunY - approxDiagonalLength},  // Diagonal top-right
        {sunX - approxDiagonalLength, sunY + approxDiagonalLength},  // Diagonal bottom-left
        {sunX + approxDiagonalLength, sunY + approxDiagonalLength}   // Diagonal bottom-right
    };

    // Draw the sunbeams
    for (int i = 0; i < 8; ++i) {
        draw_line(framebuffer, sunX, sunY, beams[i][0], beams[i][1], 0xFFFF00);
    }


    // Grass
    for (int x = 50; x <= 350; x += 10) {
        draw_line(framebuffer, x, 350, x, 370, 0x008000); // Green grass lines
    }

    // Fence
    int fenceStart = 50, fenceEnd = 350, fenceHeight = 330;
    for (int x = fenceStart; x <= fenceEnd; x += 20) {
        draw_line(framebuffer, x, fenceHeight, x, 350, 0x654321); // Vertical lines for fence
    }
    draw_line(framebuffer, fenceStart, fenceHeight, fenceEnd, fenceHeight, 0x654321); // Top horizontal line of fence
    draw_line(framebuffer, fenceStart, 340, fenceEnd, 340, 0x654321); // Bottom horizontal line of fence
}