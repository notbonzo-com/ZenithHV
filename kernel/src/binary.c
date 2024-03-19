#include <binary.h>
#include <stdint.h>


int abs(int value) {
    return (value < 0) ? -value : value;
}
bool islower(char chr)
{
    return chr >= 'a' && chr <= 'z';
}

char toupper(char chr)
{
    return islower(chr) ? (chr - 'a' + 'A') : chr;
}

void acquire_lock(k_lock *lock)
{
    while( atomic_flag_test_and_set_explicit(&lock->lock, memory_order_acquire) )
    {
        __builtin_ia32_pause();
    }
}

void release_lock(k_lock *lock) {
    atomic_flag_clear_explicit(&lock->lock, memory_order_release);
}