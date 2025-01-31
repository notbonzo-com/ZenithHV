//
// Created by notbonzo on 1/31/25.
//
// @note: This is an inline file, it will be included by intr.c
//        These header includes are only for the sake of the LSP
#include <arch/x86_64/intr.h>
#include <common/printf.h>
#include <string.h>

[[gnu::unused]] bool kpanic_inl_guard;

static void capture_regs( struct registers_ctx *context ) {
    __asm__ volatile (
        "movq %%rax, %0\n\t"
        "movq %%rbx, %1\n\t"
        "movq %%rcx, %2\n\t"
        "movq %%rdx, %3\n\t"
        "movq %%rsi, %4\n\t"
        "movq %%rdi, %5\n\t"
        "movq %%rbp, %6\n\t"
        "movq %%r8,  %7\n\t"
        "movq %%r9,  %8\n\t"
        "movq %%r10, %9\n\t"
        "movq %%r11, %10\n\t"
        "movq %%r12, %11\n\t"
        "movq %%r13, %12\n\t"
        "movq %%r14, %13\n\t"
        "movq %%r15, %14\n\t"
        : "=m" (context->rax), "=m" (context->rbx), "=m" (context->rcx), "=m" (context->rdx),
        "=m" (context->rsi), "=m" (context->rdi), "=m" (context->rbp), "=m" (context->r9),
        "=m" (context->r9), "=m" (context->r10), "=m" (context->r11), "=m" (context->r12),
        "=m" (context->r13), "=m" (context->r14), "=m" (context->r15)
        :
        : "memory"
    );

    __asm__ volatile (
        "movq %%cs,  %0\n\t"
        "movq %%ss,  %1\n\t"
        "movq %%es,  %2\n\t"
        "movq %%ds,  %3\n\t"
        "movq %%cr0, %4\n\t"
        "movq %%cr2, %5\n\t"
        "movq %%cr3, %6\n\t"
        "movq %%cr4, %7\n\t"
        : "=r" (context->cs), "=r" (context->ss), "=r" (context->es), "=r" (context->ds),
        "=r" (context->cr0), "=r" (context->cr2), "=r" (context->cr3), "=r" (context->cr4)
        :
        : "memory"
    );

    __asm__ volatile (
        "movq %%rsp, %0\n\t"
        "pushfq\n\t"
        "popq %1\n\t"
        : "=r" (context->rsp), "=r" (context->rflags)
        :
        : "memory"
    );

    context->rip = (uint64_t)__builtin_return_address(0);
}

static void print_register( const char *name, uint64_t value ) {
    printf( "%-4s: 0x%016llx", name, value ); 
}

void kpanic( struct registers_ctx* ctx, const char *fmt, ... ) {
    struct registers_ctx regs;
    if ( ctx == nullptr ) {
        capture_regs( &regs );
        regs.error_code = 0xDEADBEEF;
        regs.interrupt_vector = 0x0;
    } else {
        memcpy( &regs, ctx, sizeof(struct registers_ctx) );
    }

    printf("\n\n");
    printf("\033[1;31m");
    printf("===============================================================================\n");
    printf("                               CORE PANIC                                      \n");
    printf("===============================================================================\n");
    printf("\033[0m\n");

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        printf("\033[1;33m[ERROR] \033[0m");
        vprintf(fmt, args);
        va_end(args);
        printf("\n\n");
    } else {
        printf("\033[1;33m[ERROR] \033[0m");
        printf("%s", strings[regs.interrupt_vector]);
        printf("\n\n");
    }

    printf("\033[1;34mRegisters Dump:\033[0m\n");
    printf("\033[1;36m-------------------------------------------------------------------------------\n\033[0m");

    print_register("\033[1;34mRAX\033[0m", regs.rax); printf("    "); print_register("\033[1;34mRBX\033[0m", regs.rbx); printf("\n");
    print_register("\033[1;34mRCX\033[0m", regs.rcx); printf("    "); print_register("\033[1;34mRDX\033[0m", regs.rdx); printf("\n");
    print_register("\033[1;34mRSI\033[0m", regs.rsi); printf("    "); print_register("\033[1;34mRDI\033[0m", regs.rdi); printf("\n");
    print_register("\033[1;34mRBP\033[0m", regs.rbp); printf("    "); print_register("\033[1;34mRSP\033[0m", regs.rsp); printf("\n");
    print_register("\033[1;34mR8 \033[0m", regs.r8);  printf("    "); print_register("\033[1;34mR9 \033[0m", regs.r9);  printf("\n");
    print_register("\033[1;34mR10\033[0m", regs.r10); printf("    "); print_register("\033[1;34mR11\033[0m", regs.r11); printf("\n");
    print_register("\033[1;34mR12\033[0m", regs.r12); printf("    "); print_register("\033[1;34mR13\033[0m", regs.r13); printf("\n");
    print_register("\033[1;34mR14\033[0m", regs.r14); printf("    "); print_register("\033[1;34mR15\033[0m", regs.r15); printf("\n");

    printf("\n\033[1;34mSegment Registers:\033[0m\n");
    printf("\033[1;36m-------------------------------------------------------------------------------\n\033[0m");
    print_register("\033[1;34mCS \033[0m", regs.cs); printf("    "); print_register("\033[1;34mSS \033[0m", regs.ss); printf("\n");
    print_register("\033[1;34mDS \033[0m", regs.ds); printf("    "); print_register("\033[1;34mES \033[0m", regs.es); printf("\n");

    printf("\n\033[1;34mControl Registers:\033[0m\n");
    printf("\033[1;36m-------------------------------------------------------------------------------\n\033[0m");
    print_register("\033[1;34mCR0\033[0m", regs.cr0); printf("    "); print_register("\033[1;34mCR2\033[0m", regs.cr2); printf("\n");
    print_register("\033[1;34mCR3\033[0m", regs.cr3); printf("    "); print_register("\033[1;34mCR4\033[0m", regs.cr4); printf("\n");

    printf("\n\033[1;34mExecution Context:\033[0m\n");
    printf("\033[1;36m-------------------------------------------------------------------------------\n\033[0m");
    print_register("\033[1;34mRIP\033[0m", regs.rip); printf("    "); print_register("\033[1;34mRFLAGS\033[0m", regs.rflags); printf("\n");

    printf("\n\033[1;31m==================================================================================\n");
    printf(" SYSTEM HALTED. A fatal error occurred, and the hypervisor cannot continue safely. \n");
    printf("==================================================================================\n\033[0m");

    cli( );
    for (;;) hlt( ); // TODO Halt other cores.
}
