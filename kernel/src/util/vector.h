#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct vector {
    uint8_t *data;
    size_t _size_used_bytes;
    size_t _size_allocated_bytes;
} vector;

void vector_init(vector *v);
void vector_free(vector *v);
void vector_push(vector *v, uint8_t *data, size_t size);
void vector_pop(vector *v, uint8_t *data, size_t size);
void vector_get(vector *v, size_t index, uint8_t *data, size_t size);
void vector_set(vector *v, size_t index, uint8_t *data, size_t size);
void vector_insert(vector *v, size_t index, uint8_t *data, size_t size);
void vector_remove(vector *v, size_t index, uint8_t *data, size_t size);
void vector_resize(vector *v, size_t new_size);
void vector_clear(vector *v);
void vector_copy(vector *dest, vector *src);
void vector_move(vector *dest, vector *src);
void vector_swap(vector *v1, vector *v2);
