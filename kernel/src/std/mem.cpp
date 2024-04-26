#include <std/mem.hpp>
#include <stddef.h>
#include <kalloc.h>
#include <kprintf.h>

void* operator new(size_t size) {
    void* ptr = kmalloc(size);
    if (!ptr) {
        debugf("Failed to allocate memory\n");
                asm volatile ("hlt");
    }
    return ptr;
}

void* operator new(size_t size, void* place) noexcept {
    (void)size;
    return place;
}

void* operator new[](size_t size) {
    return operator new(size);
}

void* operator new[](size_t size, void* place) noexcept {
    return operator new(size, place);
}

void operator delete(void* ptr) noexcept {
    if (ptr) {
        kfree(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    operator delete[](ptr);
}

void operator delete(void* /* memory */, void* /* ptr */) noexcept { }
void operator delete[](void* /* memory */, void* /* ptr */) noexcept { }

namespace std {

template <typename T>
unique_ptr<T>::unique_ptr(T* p) : ptr(p) {}

template <typename T>
unique_ptr<T>::~unique_ptr() { delete ptr; }

template <typename T>
unique_ptr<T>::unique_ptr(unique_ptr&& moving) noexcept : ptr(moving.ptr) { moving.ptr = nullptr; }

template <typename T>
unique_ptr<T>& unique_ptr<T>::operator=(unique_ptr&& moving) noexcept {
    if (this != &moving) {
        delete ptr;
        ptr = moving.ptr;
        moving.ptr = nullptr;
    }
    return *this;
}

template <typename T>
T& unique_ptr<T>::operator*() const { return *ptr; }

template <typename T>
T* unique_ptr<T>::operator->() const { return ptr; }

template <typename T>
T* unique_ptr<T>::get() const { return ptr; }

template <typename T>
T* unique_ptr<T>::release() noexcept {
    T* temp = ptr;
    ptr = nullptr;
    return temp;
}

template <typename T>
void unique_ptr<T>::reset(T* p) noexcept {
    T* old = ptr;
    ptr = p;
    delete old;
}

template <typename T>
bool unique_ptr<T>::operator!() const { return ptr == nullptr; }

template <typename T>
unique_ptr<T>::operator bool() const { return ptr != nullptr; }


template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(static_cast<Args&&>(args)...));
}

}