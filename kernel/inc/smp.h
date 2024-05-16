#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t lock;
} klock;

void acquire_lock(klock* lock);
void release_lock(klock* lock);

#ifdef __cplusplus
}
#endif