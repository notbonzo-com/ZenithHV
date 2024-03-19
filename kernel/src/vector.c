#include <vector.h>
#include <mm/kalloc.h>
#include <mem.h>
#include <binary.h>

static inline void ensureVectorCapacity(vector_t* vec, size_t len);

void initVector(vector_t* vec)
{
    vec->data = (uint8_t*)kmalloc(sizeof(size_t));
    vec->capacity = sizeof(size_t);
    vec->size = 0;
}

void appendVector(vector_t* vec, const void* value, size_t len)
{
    ensureVectorCapacity(vec, len);
    memcpy(vec->data + vec->size, value, len);
    vec->size += len;
}

void freeVector(vector_t* vec)
{
    kfree(vec->data);
    initVector(vec);
}

void cleanupVector(vector_t* vec)
{
    kfree(vec->data);
    vec->data = NULL;
    vec->capacity = vec->size = 0;
}

void fillRepVector(vector_t* vec, const void* value, size_t len)
{
    for (size_t i = 0; i < vec->capacity; i += len)
        memcpy(vec->data + i, value, len);
    vec->size = vec->capacity;
}

inline void fillVector(vector_t* vec, uint8_t value)
{
    memset(vec->data, value, vec->capacity);
    vec->size = vec->capacity;
}

inline void resizeVector(vector_t* vec, size_t newCapacity)
{
    vec->data = krealloc(vec->data, newCapacity);
    vec->capacity = newCapacity;
}

int findRepVector(vector_t* vec, const void* value, size_t len)
{
    for (size_t i = 0; i < vec->size; i += len)
        if (!memcmp(vec->data + i, value, len))
            return i;
    return -1;
}

int findVector(vector_t* vec, uint8_t value)
{
    for (size_t i = 0; i < vec->size; i++)
        if (vec->data[i] == value)
            return i;
    return -1;
}

static inline void ensureVectorCapacity(vector_t* vec, size_t len)
{
    if (vec->capacity < vec->size + len)
        resizeVector(vec, vec->capacity * 2);
}