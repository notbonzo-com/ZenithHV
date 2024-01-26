#include "vector.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <drivers/debug/e9.h>
#include <drivers/mm/kmalloc.h>

void vector_init(vector *v)
{
    v->data = (uint8_t *)kalloc(sizeof(size_t));
    v->_size_used_bytes = sizeof(size_t);
    v->_size_allocated_bytes = 0;
}
void vector_free(vector *v)
{
    kfree(v->data);
    v->_size_used_bytes = 0;
    v->_size_allocated_bytes = 0;
}
void vector_push(vector *v, uint8_t *data, size_t size)
{
    if (v->_size_allocated_bytes < v->_size_used_bytes + size)
    {
        v->data = (uint8_t *)krealloc(v->data, v->_size_allocated_bytes + size);
        v->_size_allocated_bytes += size;
    }
    memcpy(v->data + v->_size_used_bytes, data, size);
    v->_size_used_bytes += size;
}
void vector_pop(vector *v, uint8_t *data, size_t size)
{
    if (v->_size_used_bytes < size)
    {
        log_warn("Vector", "vector_pop: vector is empty\n");
        return;
    }
    memcpy(data, v->data + v->_size_used_bytes - size, size);
    v->_size_used_bytes -= size;
}
void vector_get(vector *v, size_t index, uint8_t *data, size_t size)
{
    if (v->_size_used_bytes < index + size)
    {
        log_warn("Vector", "vector_get: index out of bounds\n");
        return;
    }
    memcpy(data, v->data + index, size);
}
void vector_set(vector *v, size_t index, uint8_t *data, size_t size)
{
    if (v->_size_used_bytes < index + size)
    {
        log_warn("Vector", "vector_set: index out of bounds\n");
        return;
    }
    memcpy(v->data + index, data, size);
}
void vector_insert(vector *v, size_t index, uint8_t *data, size_t size)
{
    if (v->_size_allocated_bytes < v->_size_used_bytes + size)
    {
        v->data = (uint8_t *)krealloc(v->data, v->_size_allocated_bytes + size);
        v->_size_allocated_bytes += size;
    }
    memmove(v->data + index + size, v->data + index, v->_size_used_bytes - index);
    memcpy(v->data + index, data, size);
    v->_size_used_bytes += size;
}
void vector_remove(vector *v, size_t index, uint8_t *data, size_t size)
{
    if (v->_size_used_bytes < index + size)
    {
        log_warn("Vector", "vector_remove: index out of bounds\n");
        return;
    }
    memcpy(data, v->data + index, size);
    memmove(v->data + index, v->data + index + size, v->_size_used_bytes - index - size);
    v->_size_used_bytes -= size;
}
void vector_resize(vector *v, size_t new_size)
{
    if (v->_size_allocated_bytes < new_size)
    {
        v->data = (uint8_t *)krealloc(v->data, new_size);
        v->_size_allocated_bytes = new_size;
    }
    v->_size_used_bytes = new_size;
}
void vector_clear(vector *v)
{
    v->_size_used_bytes = 0;
}
void vector_copy(vector *dest, vector *src)
{
    if (dest->_size_allocated_bytes < src->_size_used_bytes)
    {
        dest->data = (uint8_t *)krealloc(dest->data, src->_size_used_bytes);
        dest->_size_allocated_bytes = src->_size_used_bytes;
    }
    memcpy(dest->data, src->data, src->_size_used_bytes);
    dest->_size_used_bytes = src->_size_used_bytes;
}
void vector_move(vector *dest, vector *src)
{
    if (dest->_size_allocated_bytes < src->_size_used_bytes)
    {
        dest->data = (uint8_t *)krealloc(dest->data, src->_size_used_bytes);
        dest->_size_allocated_bytes = src->_size_used_bytes;
    }
    memcpy(dest->data, src->data, src->_size_used_bytes);
    dest->_size_used_bytes = src->_size_used_bytes;
    src->_size_used_bytes = 0;
}
void vector_swap(vector *v1, vector *v2)
{
    vector temp;
    vector_init(&temp);
    vector_move(&temp, v1);
    vector_move(v1, v2);
    vector_move(v2, &temp);
    vector_free(&temp);
}
