//
// Created by notbonzo on 1/30/25.
//

#ifndef BITSET_HH
#define BITSET_HH

#include <stdint.h>
#include <stddef.h>

namespace std {

    template <size_t N>
    class bitset {
    private:
        static constexpr size_t BITS_PER_UNIT = 8 * sizeof(uint64_t);
        static constexpr size_t ARRAY_SIZE = (N + BITS_PER_UNIT - 1) / BITS_PER_UNIT;

        uint64_t data[ARRAY_SIZE] = {0};

        static constexpr size_t index(size_t pos) noexcept {
            return pos / BITS_PER_UNIT;
        }

        static constexpr size_t offset(size_t pos) noexcept {
            return pos % BITS_PER_UNIT;
        }

    public:
        constexpr bitset() noexcept = default;

        constexpr void set(size_t pos, bool value = true) noexcept {
            if (pos >= N) return;
            if (value) {
                data[index(pos)] |= (uint64_t(1) << offset(pos));
            } else {
                data[index(pos)] &= ~(uint64_t(1) << offset(pos));
            }
        }

        constexpr void reset(size_t pos) noexcept {
            if (pos < N) {
                data[index(pos)] &= ~(uint64_t(1) << offset(pos));
            }
        }

        constexpr void flip(size_t pos) noexcept {
            if (pos < N) {
                data[index(pos)] ^= (uint64_t(1) << offset(pos));
            }
        }

        constexpr bool test(size_t pos) const noexcept {
            return (pos < N) && ((data[index(pos)] & (uint64_t(1) << offset(pos))) != 0);
        }

        constexpr bool all() const noexcept {
            for (size_t i = 0; i < N; ++i) {
                if (!test(i)) return false;
            }
            return true;
        }

        constexpr bool any() const noexcept {
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                if (data[i] != 0) return true;
            }
            return false;
        }

        constexpr bool none() const noexcept {
            return !any();
        }

        constexpr void reset_all() noexcept {
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                data[i] = 0;
            }
        }

        constexpr void set_all() noexcept {
            for (size_t i = 0; i < ARRAY_SIZE; ++i) {
                data[i] = ~uint64_t(0);
            }
        }

        constexpr size_t size() const noexcept {
            return N;
        }

        constexpr unsigned long to_ulong() const noexcept {
            unsigned long ret = 0;
            for (size_t i = 0; i < N; ++i) {
                ret |= (uint64_t(1) << offset(i)) * data[i];
            }
            return ret;
        }
    };

}

#endif //BITSET_HH
