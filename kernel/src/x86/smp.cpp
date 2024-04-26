#include <x86/smp.h>
#include <x86/io.h>

/**
 * Local puts -> Prints to the e9 debug port (0xe9).
 * Used as a non-locked local print function in case of a deadlock.
*/
static void localPuts(const char* str)
{
    while (*str) {
        outb(0xe9, *str++);
    }
}

extern "C" void acquire_lock(klockc *lock) {
    uint64_t loc_ticks = 0;
    uint64_t flags;
    asm volatile("pushfq; cli; popq %0" : "=r"(flags) :: "memory");

    while (!__sync_bool_compare_and_swap(&lock->locked, 0, 1)) {
        asm volatile("pause");
        if (++loc_ticks > 10000000ul) {
            localPuts("SMP: DEADLOCK In acquire_lock!\n");
            asm volatile("cli");
            asm volatile("hlt");
            // TODO: Implement kernel panic
        }
    }
    lock->flags = flags;
}

extern "C" void release_lock(klockc *lock) {
    uint64_t flags = lock->flags;

    __atomic_store_n(&lock->locked, 0, __ATOMIC_RELEASE);
    asm volatile("pushq %0; popfq" :: "r"(flags) : "memory");
}


void klock::acquire() {
    uint64_t loc_ticks = 0;
    uint64_t localFlags;
    asm volatile(
        "pushfq \n\t"
        "popq %0 \n\t"
        "cli"
        : "=r"(localFlags) // Output to local variable
        :                  // No input
        : "memory"         // Clobbers memory
    );

    flags = localFlags;

    while (!__sync_bool_compare_and_swap(&locked, 0, 1)) {
        asm volatile("pause");
        if (++loc_ticks > 10000000ul) {
            localPuts("SMP: DEADLOCK In acquire_lock!\n");
            asm volatile("cli");
            asm volatile("hlt");
            // TODO: Implement kernel panic
        }
    }
}

void klock::release() {
        uint64_t localFlags = flags;
        asm volatile(
            "pushq %0 \n\t"
            "popfq"
            :
            : "r"(localFlags)  // Input from local variable
            : "memory"          // Clobbers memory
        );


    __atomic_store_n(&locked, 0, __ATOMIC_RELEASE);
}