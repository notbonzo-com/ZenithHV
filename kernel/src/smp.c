#include <smp.h>

void acquire_lock(klock* lock)
{
	uint8_t key = 1;
    __asm__ __volatile__(
        "xchg %0, %1"
        : "=r" (key), "=m" (lock->lock)
        : "0" (key), "m" (lock->lock)
        : "memory"
    );
    while (key != 0) {
        __asm__ __volatile__(
            "xchg %0, %1"
            : "=r" (key), "=m" (lock->lock)
            : "0" (key), "m" (lock->lock)
            : "memory"
        );
    }
}

void release_lock(klock* lock)
{
	__asm__ __volatile__(
        "movb $0, %0"
        : "=m" (lock->lock)
        :
        : "memory"
    );
}