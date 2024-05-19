#include <string>
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }
 
    return dest;
}
 
void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;
 
    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }
 
    return s;
}
 
void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;
 
    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }
 
    return dest;
}
 
int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
 
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }
 
    return 0;
}

size_t strlen(const char* s) {
    size_t length = 0;
    while (*s++) {
        ++length;
    }
    return length;
}

char* strcpy(char* dest, const char* src) {
    char* saved = dest;
    while ((*dest++ = *src++));
    return saved;
}

#ifdef STRING
class string {
private:
    char* data;
    size_t length;

public:
    // Constructor
    string() : data(nullptr), length(0) {}

    // Constructor with initial value
    string(const char* str) {
        length = strlen(str);
        data = (char*) kmalloc(length + 1);
        if (data) {
            memcpy(data, str, length);
            data[length] = '\0';
        } else {
            throw std::bad_alloc();
        }
    }

    // Copy constructor
    string(const string& other) {
        length = other.length;
        data = (char*) kmalloc(length + 1);
        if (data) {
            memcpy(data, other.data, length);
            data[length] = '\0';
        } else {
            throw std::bad_alloc();
        }
    }

    // Destructor
    ~string() {
        kfree(data);
    }

    // Assignment operator
    string& operator=(const string& other) {
        if (this != &other) {
            char* newData = (char*) kmalloc(other.length + 1);
            if (newData) {
                memcpy(newData, other.data, other.length);
                newData[other.length] = '\0';

                kfree(data);
                data = newData;
                length = other.length;
            } else {
                throw std::bad_alloc();
            }
        }
        return *this;
    }

    // Append function
    void append(const char* str) {
        size_t newLength = length + strlen(str);
        char* newData = (char*) realloc(data, newLength + 1);
        if (newData) {
            memcpy(newData + length, str, strlen(str));
            newData[newLength] = '\0';
            data = newData;
            length = newLength;
        } else {
            throw std::bad_alloc();
        }
    }

    // Get length of the string
    size_t size() const {
        return length;
    }

    // C-style string access
    const char* c_str() const {
        return data;
    }
};
#endif
