// src/runtime_stubs.cpp — freestanding C++ runtime support for OS Prototype 3.0.0
#include <stddef.h>
#include "types.hpp"

using size_t_ = decltype(sizeof(0));

// Single-threaded kernel: the ABI guard functions are trivially satisfied.
extern "C" int __cxa_guard_acquire(long long *) { return 1; }
extern "C" void __cxa_guard_release(long long *) {}
extern "C" void __cxa_guard_abort(long long *) {}
extern "C" int atexit(void (*)()) { return 0; }

// Pure-virtual call trap (called when a pure virtual is invoked)
extern "C" void __cxa_pure_virtual()
{
    for (;;)
        asm volatile("hlt");
}

// ---- freestanding allocator (bump allocator over static kernel heap) ----
static unsigned char heap[16 * 1024]; // 16 KB heap: keeps total .bss under the 64 KiB real-mode segment limit
static unsigned long heap_pos = 0;

extern "C" void *sbrk_like(unsigned long size)
{
    if (heap_pos + size > sizeof(heap))
        return nullptr;
    void *p = &heap[heap_pos];
    heap_pos += (size + 15u) & ~15ul; // 16-byte align
    return p;
}

void *operator new(size_t_ size)
{
    return sbrk_like(size);
}
void *operator new[](size_t_ size)
{
    return sbrk_like(size);
}
void operator delete(void *) noexcept {}
void operator delete[](void *) noexcept {}
void operator delete(void *, size_t_) noexcept {}
void operator delete[](void *, size_t_) noexcept {}
