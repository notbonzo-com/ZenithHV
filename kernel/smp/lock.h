//
// Created by notbonzo on 1/30/25.
//

#ifndef LOCK_H
#define LOCK_H

#include <stdatomic.h>
#include <stdint.h>

typedef struct {
    atomic_flag locked;
} spinlock_t;

typedef struct {
    spinlock_t spinlock;
    uint64_t flags;
} irq_lock_t;

void spinlock_init( spinlock_t *lock );
void spinlock_lock( spinlock_t *lock );
void spinlock_unlock( spinlock_t *lock );
int spinlock_trylock( spinlock_t *lock );

void irq_lock_init( irq_lock_t *lock );
void irq_lock( irq_lock_t *lock );
void irq_unlock( irq_lock_t *lock );

#endif //LOCK_H
