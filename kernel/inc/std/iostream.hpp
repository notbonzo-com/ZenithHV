// iostream.hpp
#pragma once

// C++ Only
#ifndef __cplusplus
#error "C++ Only"
#endif

#include <stddef.h>

namespace std {

class KOutStream {
public:
    KOutStream& operator<<(int value);
    KOutStream& operator<<(unsigned int value);
    KOutStream& operator<<(const char* str);
    KOutStream& operator<<(const char value);
    KOutStream& operator<<(const bool value);
    KOutStream& operator<<(short value);
    KOutStream& operator<<(unsigned short value);
    KOutStream& operator<<(long value);
    KOutStream& operator<<(unsigned long value);
    KOutStream& operator<<(long long value);
    KOutStream& operator<<(unsigned long long value);
};

extern KOutStream cout;
}