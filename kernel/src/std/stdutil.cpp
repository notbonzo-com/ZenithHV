#include <utility>
#include <new>

namespace std
{
	void* memcpy(void* dest, const void* src, size_t n) {
		uint8_t* d = static_cast<uint8_t*>(dest);
		const uint8_t* s = static_cast<const uint8_t*>(src);
		for (size_t i = 0; i < n; ++i) {
			d[i] = s[i];
		}
		return dest;
	}


	void* memmove(void* dest, const void* src, size_t n) {
		uint8_t* d = static_cast<uint8_t*>(dest);
		const uint8_t* s = static_cast<const uint8_t*>(src);
		if (d < s) {
			for (size_t i = 0; i < n; ++i) {
				d[i] = s[i];
			}
		} else if (d > s) {
			for (size_t i = n; i > 0; --i) {
				d[i - 1] = s[i - 1];
			}
		}
		return dest;
	}
	
	void* memset(void* dest, int val, size_t n) {
		uint8_t* d = static_cast<uint8_t*>(dest);
		for (size_t i = 0; i < n; ++i) {
			d[i] = static_cast<uint8_t>(val);
		}
		return dest;
	}

	int memcmp(const void* ptr1, const void* ptr2, size_t num) {
		const uint8_t* p1 = static_cast<const uint8_t*>(ptr1);
		const uint8_t* p2 = static_cast<const uint8_t*>(ptr2);
		for (size_t i = 0; i < num; ++i) {
			if (p1[i] != p2[i]) {
				return (p1[i] < p2[i]) ? -1 : 1;
			}
		}
		return 0;
	}


	void Bitmap::set(size_t bit_index) {
		if (bit_index < size * 8) {
			bitmap[bit_index / 8] |= (1 << (bit_index % 8));
		}
	}
	void Bitmap::unset(size_t bit_index) {
		if (bit_index < size * 8) {
			bitmap[bit_index / 8] &= ~(1 << (bit_index % 8));
		}
	}
	bool Bitmap::read(size_t bit_index) const {
		if (bit_index < size * 8) {
			return bitmap[bit_index / 8] & (1 << (bit_index % 8));
		}
		return false;
	}
	size_t Bitmap::bit_size() const {
		return size * 8;
	}
	ssize_t Bitmap::find_first_free() const {
		for (size_t byte = 0; byte < size; ++byte) {
			if (bitmap[byte] != 0xFF) {
				for (size_t bit = 0; bit < 8; ++bit) {
					if (!(bitmap[byte] & (1 << bit))) {
						return byte * 8 + bit;
					}
				}
			}
		}
		return -1;
	}
	size_t Bitmap::count_set() const {
		size_t count = 0;
		for (size_t byte = 0; byte < size; ++byte) {
			count += __builtin_popcount(bitmap[byte]);
		}
		return count;
	}
	size_t Bitmap::count_unset() const {
		return bit_size() - count_set();
	}
	void Bitmap::resize(uint8_t* new_bitmap, size_t new_size) {
		if (new_size < size) {
			std::memcpy(new_bitmap, bitmap, new_size);
		} else {
			std::memcpy(new_bitmap, bitmap, size);
			std::memset(new_bitmap + size, 0, new_size - size);
		}
		bitmap = new_bitmap;
		size = new_size;
	}
	uint8_t* Bitmap::data() {
		return bitmap;
	}
	size_t Bitmap::byte_size() const {
		return size;
	}

	char* strcpy(char* dest, const char* src) {
        char* original_dest = dest;
        while ((*dest++ = *src++));
        return original_dest;
    }

    char* strncpy(char* dest, const char* src, size_t n) {
        char* original_dest = dest;
        while (n && (*dest++ = *src++)) {
            n--;
        }
        while (n--) {
            *dest++ = '\0';
        }
        return original_dest;
    }

    size_t strlen(const char* str) {
        const char* s = str;
        while (*s) {
            ++s;
        }
        return s - str;
    }

    int strcmp(const char* str1, const char* str2) {
        while (*str1 && (*str1 == *str2)) {
            ++str1;
            ++str2;
        }
        return *(unsigned char*)str1 - *(unsigned char*)str2;
    }

    int strncmp(const char* str1, const char* str2, size_t n) {
        while (n && *str1 && (*str1 == *str2)) {
            ++str1;
            ++str2;
            --n;
        }
        if (n == 0) {
            return 0;
        } else {
            return *(unsigned char*)str1 - *(unsigned char*)str2;
        }
    }

    char* strcat(char* dest, const char* src) {
        char* original_dest = dest;
        while (*dest) {
            ++dest;
        }
        while ((*dest++ = *src++));
        return original_dest;
    }

    char* strncat(char* dest, const char* src, size_t n) {
        char* original_dest = dest;
        while (*dest) {
            ++dest;
        }
        while (n-- && (*dest++ = *src++));
        if (n == 0) {
            *dest = '\0';
        }
        return original_dest;
    }

    const char* strchr(const char* str, int c) {
        while (*str != (char)c) {
            if (*str == '\0') {
                return nullptr;
            }
            ++str;
        }
        return str;
    }

    const char* strrchr(const char* str, int c) {
        const char* last = nullptr;
        do {
            if (*str == (char)c) {
                last = str;
            }
        } while (*str++);
        return last;
    }

    char* itoa(int value, char* str, int base) {
        char* ptr = str;
        char* ptr1 = str;
        char tmp_char;
        int tmp_value;

        if (base < 2 || base > 36) {
            *str = '\0';
            return str;
        }

        if (value < 0 && base == 10) {
            *ptr++ = '-';
        }

        do {
            tmp_value = value;
            value /= base;
            *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"
                    [35 + (tmp_value - value * base)];
        } while (value);

        if (*str == '-') {
            ptr1++;
        }

        *ptr-- = '\0';

        while (ptr1 < ptr) {
            tmp_char = *ptr;
            *ptr-- = *ptr1;
            *ptr1++ = tmp_char;
        }

        return str;
    }

        long int strtol(const char* str, char** endptr, int base) {
        const char* s = str;
        long int result = 0;
        int sign = 1;

        while (*s == ' ' || *s == '\t') {
            s++;
        }

        if (*s == '-') {
            sign = -1;
            s++;
        } else if (*s == '+') {
            s++;
        }

        if ((base == 0 || base == 16) &&
            *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
            s += 2;
            base = 16;
        } else if (base == 0 && *s == '0') {
            base = 8;
        } else if (base == 0) {
            base = 10;
        }

        while (*s) {
            int digit = *s - '0';
            if (digit >= 0 && digit < base) {
                result = result * base + digit;
            } else if (base > 10 && *s >= 'a' && *s <= 'f') {
                result = result * base + (*s - 'a' + 10);
            } else if (base > 10 && *s >= 'A' && *s <= 'F') {
                result = result * base + (*s - 'A' + 10);
            } else {
                break;
            }
            s++;
        }

        if (endptr) {
            *endptr = const_cast<char*>(s);
        }

        return sign * result;
    }

    unsigned long int strtoul(const char* str, char** endptr, int base) {
        const char* s = str;
        unsigned long int result = 0;

        while (*s == ' ' || *s == '\t') {
            s++;
        }

        if (*s == '+') {
            s++;
        }

        if ((base == 0 || base == 16) &&
            *s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
            s += 2;
            base = 16;
        } else if (base == 0 && *s == '0') {
            base = 8;
        } else if (base == 0) {
            base = 10;
        }

        while (*s) {
            int digit = *s - '0';
            if (digit >= 0 && digit < base) {
                result = result * base + digit;
            } else if (base > 10 && *s >= 'a' && *s <= 'f') {
                result = result * base + (*s - 'a' + 10);
            } else if (base > 10 && *s >= 'A' && *s <= 'F') {
                result = result * base + (*s - 'A' + 10);
            } else {
                break;
            }
            s++;
        }

        if (endptr) {
            *endptr = const_cast<char*>(s);
        }

        return result;
    }

    int atoi(const char* str) {
        return static_cast<int>(strtol(str, nullptr, 10));
    }
} // namespace std

void __cpuid(uint32_t code, uint32_t &eax, uint32_t &ebx, uint32_t &ecx, uint32_t &edx) {
    asm volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (code), "c" (0)
    );
}
