#pragma once

#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

template <typename F>
struct Defer_Struct {
    F f;
    Defer_Struct(F f) : f(f) {}
    ~Defer_Struct() { f(); }
};

template <typename F>
inline Defer_Struct <F> make_defer_func(F f) {
    return Defer_Struct <F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __LINE__)
#define defer(code)   auto DEFER_3(_defer_) = make_defer_func([&](){code;})

#define ArrayCount(arr) (sizeof(arr)/sizeof((arr)[0]))
#define Max(x, y) ((x) > (y) ? (x) : (y))
#define Min(x, y) ((x) < (y) ? (x) : (y))

#define Bit(x) (1 << (x))
