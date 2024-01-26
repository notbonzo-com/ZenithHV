#include "main.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <util/binary.h>
#include <drivers/debug/e9.h>

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