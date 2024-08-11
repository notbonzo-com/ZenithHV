#include <utility>
#include <new>
#include <cstring>

namespace std
{
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
} // namespace std

void __cpuid(uint32_t code, uint32_t &eax, uint32_t &ebx, uint32_t &ecx, uint32_t &edx) {
    asm volatile (
        "cpuid"
        : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
        : "a" (code), "c" (0)
    );
}
