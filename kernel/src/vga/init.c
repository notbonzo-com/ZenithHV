#include <vga/init.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <binary.h>
#include <debug.h>

LIMINE_BASE_REVISION(1)

struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

framebuffer_t* g_Framebuffer;

void VISUAL_Init() {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        log_crit("Kernel Initiliser", "Limine revision not supported");
        asm volatile ("cli; hlt");
    }
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        log_crit("Kernel Initiliser", "No framebuffer found recived from Limine");
        asm volatile ("cli; hlt");
    }
    g_Framebuffer = framebuffer_request.response->framebuffers[0];
}

void VISUAL_SetPixel(uint32_t x, uint32_t y, uint32_t color, uint8_t value) {
    if (x >= g_Framebuffer->width || y >= g_Framebuffer->height)
        return;

    uint32_t fb_pos = (y * g_Framebuffer->pitch) + (x * (g_Framebuffer->bpp / 8));

    uint32_t *pixel = (uint32_t *)((uintptr_t)g_Framebuffer->address + fb_pos);
    *pixel = value ? color : 0;
}

void VISUAL_ClrScr()
{
    for (uint64_t y = 0; y < g_Framebuffer->height; y++)
    {
        for (uint64_t x = 0; x < g_Framebuffer->width; x++)
        {
            VISUAL_SetPixel(x, y, 0, 0);
        }
    }
}