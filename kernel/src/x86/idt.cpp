#include <x86/idt.h>
#include <kprintf.h>
#include <x86/smp.h>
#include <stdbool.h>

IDTPointer_t idptr;

__attribute__((aligned(16))) IDTEntry_t idt[256];
extern uint64_t commonHandlerJMPTable[];
uintptr_t handlerTable[256] = {0};

struct stacktrace_tree {
    struct stacktrace_tree *rbp;
    uintptr_t rsp;
};

void stacktrace(IDTContext_t *regs) {
    kprintf("┌────────────────────────────────────────────────┐\n");
    kprintf("│   Stack Trace                                  │\n");

    uint64_t *rbp = (uint64_t *)regs->rbp;
    uint64_t *rip;

    asm volatile ("movq %%rbp, %0" : "=r"(rbp) ::);

    while (rbp != NULL && (rip = (rbp + 1)) != 0) {
        kprintf("├────────────────────────────────────────────────┤\n");
        kprintf("│ RIP: 0x%016lx                        │\n", rip);
        rbp = (uint64_t *)*rbp;
        if (!rbp) break;
    }

    kprintf("└────────────────────────────────────────────────┘\n");
}

void IDT_SetDescriptor(uint8_t vector, uintptr_t isr, uint8_t flags)
{
    IDTEntry_t *descriptor = &idt[vector];

    descriptor->offset_low = isr & 0xFFFF;
    descriptor->selector = 0x8;
    descriptor->zero = 0;
    descriptor->type_attr = flags;
    descriptor->offset_mid = (isr >> 16) & 0xFFFF;
    descriptor->offset_hi = (isr >> 32) & 0xFFFFFFFF;
    descriptor->zero2 = 0;
}
void initIDT()
{
    for (size_t vector = 0; vector <= 255; vector++) {
        IDT_SetDescriptor(vector, commonHandlerJMPTable[vector], 0x8E);
    }
    idptr.offset = (uintptr_t)idt;
    idptr.size = (uint16_t)(sizeof(idt) - 1);

    asm volatile (
        "lidt %0"
        : : "m"(idptr) : "memory"
    );
}

struct ErrorCodeMap {
    size_t error_code;
    const char *error_string;
    size_t print_error_code;
};

static const char* cpu_exception_strings[32] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable-Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid opcode",
    "Device (FPU) not available",
    "Double Fault",
    "RESERVED VECTOR",
    "Invalid TSS, ",
    "Segment not present, ",
    "Stack Segment Fault, ",
    "General Protection Fault, ",
    "Page Fault, ",
    "RESERVED VECTOR",
    "x87 FP Exception",
    "Alignment Check, ",
    "Machine Check (Internal Error)",
    "SIMD FP Exception",
    "Virtualization Exception",
    "Control  Protection Exception, ",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "Hypervisor Injection Exception",
    "VMM Communication Exception, ",
    "Security Exception, ",
    "RESERVED VECTOR"
};

static void print_register(const char *name, uint64_t value) {
    kprintf("%-4s: 0x%016llx", name, value); 
}

void default_exception_handler(IDTContext_t *regs)
{
    size_t error_code = regs->error_code;

    kprintf("----------------------------------------------------------\n");
    kprintf("Exception: %lu -> %s\n", regs->vector, regs->vector < 32 ? 
        cpu_exception_strings[regs->vector] : "?");
    kprintf("----------------------------------------------------------\n");
    kprintf("Error code: 0x%016llx\n", error_code);
    

    kprintf("┌───────────┬────────────────────────────────────────┐\n");
    kprintf("│   Register│ Value                                  │\n");
    kprintf("├───────────┼────────────────────────────────────────┤\n");
    print_register("│       CR0 ", regs->cr0); kprintf("                     │\n");
    print_register("│       CR2 ", regs->cr2); kprintf("                     │\n");
    print_register("│       CR3 ", regs->cr3); kprintf("                     │\n");
    print_register("│       CR4 ", regs->cr4); kprintf("                     │\n");
    print_register("│       RAX ", regs->rax); kprintf("                     │\n");
    print_register("│       RBX ", regs->rbx); kprintf("                     │\n");
    print_register("│       RCX ", regs->rcx); kprintf("                     │\n");
    print_register("│       RDX ", regs->rdx); kprintf("                     │\n");
    print_register("│       RSI ", regs->rsi); kprintf("                     │\n");
    print_register("│       RDI ", regs->rdi); kprintf("                     │\n");
    print_register("│       RBP ", regs->rbp); kprintf("                     │\n");
    print_register("│       RSP ", regs->rsp); kprintf("                     │\n");
    print_register("│       R8  ", regs->r8); kprintf("                     │\n");
    print_register("│       R9  ", regs->r9); kprintf("                     │\n");
    print_register("│       R10 ", regs->r10);kprintf("                     │\n");
    print_register("│       R11 ", regs->r11);kprintf("                     │\n");
    print_register("│       R12 ", regs->r12);kprintf("                     │\n");
    print_register("│       R13 ", regs->r13);kprintf("                     │\n");
    print_register("│       R14 ", regs->r14);kprintf("                     │\n");
    print_register("│       R15 ", regs->r15);kprintf("                     │\n");
    // kprintf("└───────────┴────────────────────────────────────────┘\n");

    kprintf("├───────────┼────────────────────────────────────────┼────────────────────┐\n");
    kprintf("│   RFLAGS  │                                        │                    │\n");
    kprintf("└───────────┴────────────────────────────────────────┴────────────────────┘\n");

    uint64_t rflags = regs->rflags;
    kprintf("CF: %d | PF: %d | AF: %d | ZF: %d | SF: %d | TF: %d | IF: %d | DF: %d | OF: %d\n \
    IOPL: %d%d | NT: %d | RF: %d | VM: %d | AC: %d | VIF: %d | VIP: %d | ID: %d\n",
            (rflags >> 0) & 1,
            (rflags >> 2) & 1,
            (rflags >> 4) & 1,
            (rflags >> 6) & 1,
            (rflags >> 7) & 1,
            (rflags >> 8) & 1,
            (rflags >> 9) & 1,
            (rflags >> 10) & 1,
            (rflags >> 11) & 1,
            (rflags >> 12) & 1, 
            (rflags >> 13) & 1,
            (rflags >> 14) & 1,
            (rflags >> 16) & 1,
            (rflags >> 17) & 1,
            (rflags >> 18) & 1,
            (rflags >> 19) & 1,
            (rflags >> 20) & 1,
            (rflags >> 21) & 1);

    stacktrace(regs);

    asm volatile ("cli");
    asm volatile ("hlt");
}


klock setvectorlock;
void IDT_SetVector(size_t vector, uintptr_t handler)
{
    debugf("Registered vector %lu at 0x%p", vector, handler);
    setvectorlock.acquire();
    debugf("Testing vector %lu at 0x%p", vector, handler);
    if (handlerTable[vector] != (uintptr_t)default_exception_handler || !handler) {
        debugf("Failed to register vector %lu at 0x%p", vector, handler);
        setvectorlock.release();
        return;
    }
    handlerTable[vector] = handler;
    setvectorlock.release();
}
void IDT_ResetVector(size_t vector)
{
    setvectorlock.acquire();
    if (handlerTable[vector] == (uintptr_t)default_exception_handler || vector < 32) {
        debugf("Failed to erase vector %lu", vector);
        __asm__ ("hlt");
    }
    handlerTable[vector] = (uintptr_t)default_exception_handler;
    setvectorlock.release();
}

void capture_regs(IDTContext_t *context) {
    asm volatile (
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
          "=m" (context->rsi), "=m" (context->rdi), "=m" (context->rbp), "=m" (context->r8),
          "=m" (context->r9), "=m" (context->r10), "=m" (context->r11), "=m" (context->r12),
          "=m" (context->r13), "=m" (context->r14), "=m" (context->r15)
        :
        : "memory"
    );

    asm volatile (
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

    asm volatile (
        "movq %%rsp, %0\n\t"
        "pushfq\n\t"
        "popq %1\n\t"
        : "=r" (context->rsp), "=r" (context->rflags)
        :
        : "memory"
    );

    context->rip = (uint64_t)__builtin_return_address(0);
}

void kpanic() {
    IDTContext_t context;
    capture_regs(&context);
    context.error_code = 0xDEADBEEF;
    context.vector = 6969;

    default_exception_handler(&context);
}