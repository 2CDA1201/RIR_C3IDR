#ifndef BIT_UTILS_HPP
#define BIT_UTILS_HPP

#include <cstdint>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace bit {

// x の最上位ビットから数えた先行ゼロのビット数を返します。
inline uint32_t count_leading_zeros(uint32_t x) noexcept
{
    if (x == 0) {
        return 32;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_clz(x);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanReverse(&index, x);
    return 31 - index;
#else
#error "Unsupported compiler"
#endif
}

// x の最下位ビットから数えた後置ゼロのビット数を返します。
inline uint32_t count_trailing_zeros(uint32_t x) noexcept
{
    if (x == 0) {
        return 32;
    }
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctz(x);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward(&index, x);
    return index;
#else
#error "Unsupported compiler"
#endif
}

} // namespace bit

#endif // BIT_UTILS_HPP