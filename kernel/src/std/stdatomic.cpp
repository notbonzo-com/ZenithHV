#include <atomic>

namespace std {

void klock::acquire() {
    uint8_t key = 1;
    __asm__ __volatile__(
        "xchg %0, %1"
        : "=r" (key), "=m" (lock)
        : "0" (key), "m" (lock)
        : "memory"
    );
    while (key != 0) {
        __asm__ __volatile__(
            "xchg %0, %1"
            : "=r" (key), "=m" (lock)
            : "0" (key), "m" (lock)
            : "memory"
        );
    }
}

void klock::release() {
    __asm__ __volatile__(
        "movb $0, %0"
        : "=m" (lock)
        :
        : "memory"
    );
}

void klock::a()
{
    this->acquire();
}

void klock::r()
{
    this->release();
}
}