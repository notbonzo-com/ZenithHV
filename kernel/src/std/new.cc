#include <stddef.h>
#include <kmalloc>
#include <sys/idt.hh>

void* operator new(size_t size) {
    void* ptr = umalloc(size);
    if (!ptr) {
        intr::kpanic(nullptr, "Failed to allocate memory");
    }
    return ptr;
}

void operator delete(void* ptr) noexcept {
    if (ptr) {
        ufree(ptr);
    }
}

void operator delete(void* ptr, size_t) noexcept {
    if (ptr) {
        ufree(ptr);
    }
}

void* operator new[](size_t size) {
    void* ptr = umalloc(size);
    if (!ptr) {
        intr::kpanic(nullptr, "Failed to allocate memory");
    }
    return ptr;
}

void operator delete[](void* ptr) noexcept {
    if (ptr) {
        ufree(ptr);
    }
}

void operator delete[](void* ptr, size_t) noexcept {
    if (ptr) {
        ufree(ptr);
    }
}

// Placement new
void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

void operator delete(void*, void*) noexcept {
    // Do nothing
}