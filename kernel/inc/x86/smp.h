#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {

class klock {
    volatile uint8_t locked = 0;
    volatile uint8_t flags = 0;
public:
    void acquire();
    void release();
};

#endif

/* C LOCKS */

typedef struct {
    volatile uint8_t locked;
    volatile uint8_t flags;
} klockc;

void acquire_lock(klockc *lock);
void release_lock(klockc *lock);



#ifdef __cplusplus
}
#endif