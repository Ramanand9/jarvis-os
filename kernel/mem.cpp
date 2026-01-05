#include <stddef.h>
#include <stdint.h>

extern "C" void *memcpy(void *dest, const void *src, size_t n) {
    auto *d = (uint8_t*)dest;
    auto *s = (const uint8_t*)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

extern "C" void *memset(void *s, int c, size_t n) {
    auto *p = (uint8_t*)s;
    for (size_t i = 0; i < n; i++) p[i] = (uint8_t)c;
    return s;
}

extern "C" void *memmove(void *dest, const void *src, size_t n) {
    auto *d = (uint8_t*)dest;
    auto *s = (const uint8_t*)src;
    if (s > d) {
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else if (s < d) {
        for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dest;
}

extern "C" int memcmp(const void *a, const void *b, size_t n) {
    auto *p = (const uint8_t*)a;
    auto *q = (const uint8_t*)b;
    for (size_t i = 0; i < n; i++) {
        if (p[i] != q[i]) return (p[i] < q[i]) ? -1 : 1;
    }
    return 0;
}

// Minimal C++ ABI stubs for freestanding kernel
extern "C" void __cxa_pure_virtual() { for(;;) asm volatile ("hlt"); }
extern "C" int __cxa_atexit(void*, void*, void*) { return 0; }
