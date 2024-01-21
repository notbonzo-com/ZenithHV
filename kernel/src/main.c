#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <util/mem.h>
#include <drivers/debug/e9.h>
#include <drivers/visual.h>
#include <hal/hal.h>

#define forever(a) forever_start: a ; goto forever_start

LIMINE_BASE_REVISION(1)

static void hcf(void) {
    asm ("cli");
    forever (
        asm ("hlt");
    );
}

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        log_crit("Kernel Initiliser", "Limine revision not supported");
        hcf();
    }
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        log_crit("Kernel Initiliser", "No framebuffer found recived from Limine");
        hcf();
    }

    HAL_Init();
    // Get the first framebuffer
    //struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    forever(asm ("hlt"));
}
