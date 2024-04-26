#include <std/iostream.hpp>
#include <kalloc.h>
#include <kprintf.h>

namespace std
{

KOutStream cout;

// Pls dont keep this shit here once you make a proper string.h


void reverse(char* str, int len) {
    int i = 0, j = len - 1;
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

template<typename T>
int numLength(T number) {
    int length = (number <= 0) ? 1 : 0;
    while (number != 0) {
        length++;
        number /= 10;
    }
    return length;
}

template<typename T>
char* int_to_str(T value, char* str, int base = 10) {
    T i = 0;
    bool isNegative = false;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        isNegative = true;
        value = -value;
    }

    while (value != 0) {
        T rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return str;
}


KOutStream& KOutStream::operator<<(int value) {
    char buffer[12];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(unsigned int value) {
    char buffer[12];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(const char* str) {
    kprintf("%s", str);
    return *this;
}

KOutStream& KOutStream::operator<<(const char value) {
    char buffer[2] = {value, '\0'};
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(const bool value) {
    kprintf(value ? "true" : "false");
    return *this;
}

KOutStream& KOutStream::operator<<(short value) {
    char buffer[7];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(unsigned short value) {
    char buffer[6];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(long value) {
    char buffer[12];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(unsigned long value) {
    char buffer[11];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(long long value) {
    char buffer[21];
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

KOutStream& KOutStream::operator<<(unsigned long long value) {
    char buffer[21]; 
    int_to_str(value, buffer, 10);
    kprintf("%s", buffer);
    return *this;
}

}