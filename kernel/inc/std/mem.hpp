// mem.hpp
#pragma once

// C++ Only
#ifndef __cplusplus
#error "C++ Only"
#endif

#include <stddef.h>

void* operator new(size_t size);
void* operator new(size_t size, void* place) noexcept;
void* operator new[](size_t size);
void* operator new[](size_t size, void* place) noexcept;

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, size_t) noexcept;
void operator delete[](void* ptr, size_t) noexcept;
void operator delete(void* memory, void* ptr) noexcept;
void operator delete[](void* memory, void* ptr) noexcept;


namespace std
{

template <typename T>
class unique_ptr {
private:
    T* ptr;

public:
    explicit unique_ptr(T* p = nullptr);
    ~unique_ptr();

    unique_ptr(unique_ptr&& moving) noexcept;
    unique_ptr& operator=(unique_ptr&& moving) noexcept;

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    T& operator*() const;
    T* operator->() const;
    T* get() const;
    T* release() noexcept;
    void reset(T* p = nullptr) noexcept;

    bool operator!() const;
    explicit operator bool() const;
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args);

}