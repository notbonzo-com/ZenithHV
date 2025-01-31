//
// Created by notbonzo on 1/30/25.
//

#include <smp/lock.h>
#include <arch/io.h>

void spinlock_init( spinlock_t *lock ) {
    atomic_flag_clear( &lock->locked );
}

void spinlock_lock( spinlock_t *lock ) {
    while ( atomic_flag_test_and_set( &lock->locked ) ) {
        pause( );
    }
}

int spinlock_trylock( spinlock_t *lock ) {
    return !atomic_flag_test_and_set( &lock->locked );
}

void spinlock_unlock( spinlock_t *lock ) {
    atomic_flag_clear( &lock->locked );
}

void irq_lock_init( irq_lock_t *lock ) {
    spinlock_init( &lock->spinlock );
    lock->flags = 0;
}

void irq_lock( irq_lock_t *lock ) {
    bool interrupts_were_enabled = is_interrupts_enabled( );

    if ( interrupts_were_enabled ) {
        __asm__ volatile ( "cli" );
    }

    spinlock_lock( &lock->spinlock );
    lock->flags = interrupts_were_enabled;
}

void irq_unlock( irq_lock_t *lock ) {
    spinlock_unlock( &lock->spinlock );

    if ( lock->flags ) {
        __asm__ volatile ( "sti" );
    } else {
        __asm__ volatile ( "cli" );
    }
}
