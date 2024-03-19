#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t *data;
    size_t size;
    size_t capacity;
} vector_t;

void initVector(vector_t *vec);
void appendVector(vector_t *vec, const void *value, size_t bytes);
void freeVector(vector_t *vec);
void cleanupVector(vector_t *vec);
void fillRepVector(vector_t *vec, const void *value, size_t bytes);
void fillVector(vector_t *vec, uint8_t value);
void resizeVector(vector_t *vec, size_t bytes);
int findRepVector(vector_t *vec, const void *value, size_t bytes);
int findVector(vector_t *vec, uint8_t value);