#include <sys/idt.hh>
#include <io>
#include <kprintf>
#include <atomic>
#include <cstring>

namespace intr {

    __attribute__((aligned(16))) entry_t entries[256];
    extern "C" {
        uintptr_t realHandler[256] = {0};
    }
    pointer_t idtr;

    extern "C" void default_interrupt_handler(regs_t* CPU)
    {
        kpanic(CPU, "?");
    }

    static const char* strings[32] = {
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
        "Invalid TSS",
        "Segment not present",
        "Stack Segment Fault",
        "General Protection Fault ",
        "Page Fault ",
        "RESERVED VECTOR",
        "x87 FP Exception",
        "Alignment Check",
        "Machine Check (Internal Error)",
        "SIMD FP Exception",
        "Virtualization Exception",
        "Control  Protection Exception",
        "RESERVED VECTOR",
        "RESERVED VECTOR",
        "RESERVED VECTOR",
        "RESERVED VECTOR",
        "RESERVED VECTOR",
        "RESERVED VECTOR",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        "RESERVED VECTOR"
    };

    void stacktrace(regs_t *regs) {
        kprintf("┌────────────────────────────────────────────────┐\n");
        kprintf("│   Stack Trace                                  │\n");

        uint64_t *rbp = (uint64_t *)regs->RBP;
        uint64_t *rip;

        asm volatile ("movq %%rbp, %0" : "=r"(rbp) ::);

        while (rbp != nullptr && (rip = (rbp + 1)) != 0) {
            kprintf("├────────────────────────────────────────────────┤\n");
            kprintf("│ RIP: 0x%016lx                        │\n", rip);
            rbp = (uint64_t *)*rbp;
            if (!rbp) break;
        }

        kprintf("└────────────────────────────────────────────────┘\n");
    }

    void setGate(uint8_t interrupt, uintptr_t base, int8_t flags)
    {
        entry_t *descriptor = &entries[interrupt];

        descriptor->offset_low = base & 0xFFFF;
        descriptor->selector = 0x8;
        descriptor->ist = 0;
        descriptor->attribute = flags;
        descriptor->offset_mid = (base >> 16) & 0xFFFF;
        descriptor->offset_high = (base >> 32) & 0xFFFFFFFF;
        descriptor->zero = 0;
    }

    void capture_regs(regs_t *context) {
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
            : "=m" (context->RAX), "=m" (context->RBX), "=m" (context->RCX), "=m" (context->RDX),
            "=m" (context->RSI), "=m" (context->RDI), "=m" (context->RBP), "=m" (context->R8),
            "=m" (context->R9), "=m" (context->R10), "=m" (context->R11), "=m" (context->R12),
            "=m" (context->R13), "=m" (context->R14), "=m" (context->R15)
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
            "=r" (context->CR0), "=r" (context->CR2), "=r" (context->CR3), "=r" (context->CR4)
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
    static void print_register(const char *name, uint64_t value) {
        kprintf("%-4s: 0x%016llx", name, value); 
    }

    void kpanic(regs_t *nregs, const char* str)
    {
        regs_t regs;
        if (nregs == nullptr)
        {
            capture_regs(&regs);
            regs.error = 0xDEADBEEF;
            regs.interrupt = 0xDEADBEEF;
        } else {
            std::memcpy(&regs, nregs, sizeof(regs_t));
        }

        kprintf("----------------------------------------------------------\n");
        kprintf("Exception: %lu > %s\n", regs.interrupt, regs.interrupt < 32 ? 
            strings[regs.interrupt] : str);
        kprintf("----------------------------------------------------------\n");
        kprintf("Error code: 0x%016llx\n", regs.error);

        if (regs.interrupt == 14)
        {
            kprintf("Fault caused by %s operation.\n", (regs.error & 0x1) ? "protection violation" : "non-present page");
            kprintf("Fault caused during a %s.\n", (regs.error & 0x2) ? "write" : "read");
            kprintf("Fault occurred in %s mode.\n", (regs.error & 0x4) ? "user" : "supervisor");
            if (regs.error & 0x8) {
                kprintf("Fault caused by a reserved write.\n");
            }
            if (regs.error & 0x10) {
                kprintf("Fault caused by an instruction fetch.\n");
            }
        }    

        kprintf("┌───────────┬────────────────────────────────────────┐\n");
        kprintf("│   Register│ Value                                  │\n");
        kprintf("├───────────┼────────────────────────────────────────┤\n");
        print_register("│       CR0 ", regs.CR0); kprintf("                     │\n");
        print_register("│       CR2 ", regs.CR2); kprintf("                     │\n");
        print_register("│       CR3 ", regs.CR3); kprintf("                     │\n");
        print_register("│       CR4 ", regs.CR4); kprintf("                     │\n");
        print_register("│       RAX ", regs.RAX); kprintf("                     │\n");
        print_register("│       RBX ", regs.RBX); kprintf("                     │\n");
        print_register("│       RCX ", regs.RCX); kprintf("                     │\n");
        print_register("│       RDX ", regs.RDX); kprintf("                     │\n");
        print_register("│       RSI ", regs.RBP); kprintf("                     │\n");
        print_register("│       RDI ", regs.RSI); kprintf("                     │\n");
        print_register("│       RBP ", regs.RDI); kprintf("                     │\n");
        print_register("│       RSP ", regs.RSI); kprintf("                     │\n");
        print_register("│       R8  ", regs.R8); kprintf("                     │\n");
        print_register("│       R9  ", regs.R9); kprintf("                     │\n");
        print_register("│       R10 ", regs.R10);kprintf("                     │\n");
        print_register("│       R11 ", regs.R11);kprintf("                     │\n");
        print_register("│       R12 ", regs.R12);kprintf("                     │\n");
        print_register("│       R13 ", regs.R13);kprintf("                     │\n");
        print_register("│       R14 ", regs.R14);kprintf("                     │\n");
        print_register("│       R15 ", regs.R15);kprintf("                     │\n");
        // kprintf("└───────────┴────────────────────────────────────────┘\n");

        kprintf("├───────────┼────────────────────────────────────────┼────────────────────┐\n");
        kprintf("│   RFLAGS  │                                        │                    │\n");

        uint64_t rflags = regs.rflags;
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
        kprintf("└───────────┴────────────────────────────────────────┴────────────────────┘\n");

        stacktrace(&regs);

        //TODO Halt other cores
        asm volatile ("cli");
        asm volatile ("hlt");
    }

    void load()
    {
        asm volatile (
            "lidt %0"
            : : "m"(idtr) : "memory"
        );
    }

    void init()
    {
        kprintf(" -> Setting vectors 0-32 to default_interrupt_handler\n");
        for (size_t vector = 0; vector < 32; vector++) {
            setGate(vector, stubs[vector], 0b10001111);
            realHandler[vector] = reinterpret_cast<uintptr_t>(default_interrupt_handler);
        }
        kprintf(" -> Setting vectors 32-256 to default_interrupt_handler\n");
        for (size_t vector = 32; vector < 256; vector++) {
            setGate(vector, stubs[vector], 0b10001110);
            realHandler[vector] = reinterpret_cast<uintptr_t>(default_interrupt_handler);
        }

        kprintf(" -> Loading the IDT Pointer into the IDTR register\n");
        kprintf("  ---> Offset: 0x%00x\n", reinterpret_cast<uintptr_t>(entries));
        idtr.offset = reinterpret_cast<uintptr_t>(entries);
        kprintf("  ---> Limit: 0x%00x\n", (uint16_t)(sizeof(entries) - 1));
        idtr.limit = static_cast<uint16_t>(sizeof(entries) - 1);

        load();
        kprintf(" -> IDT initialization complete\n");
    }

    std::klock regvec_lock;

    bool register_handler(size_t vector, handler_t handler)
    {
        std::auto_lock regvec_alock(regvec_lock);
        if (realHandler[vector] == reinterpret_cast<uintptr_t>(default_interrupt_handler)) {
            realHandler[vector] = reinterpret_cast<uintptr_t>(handler);
            return true;
        }
        return false;
    }

}