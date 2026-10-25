// include/port8.hpp — RAII 8-bit I/O port (the "8-bit hardware" abstraction)
#pragma once
#include "types.hpp"

namespace hw
{

    class Port8
    {
    public:
        constexpr explicit Port8(u16 io) : _io(io) {}

        [[nodiscard]] u8 read() const
        {
            u8 v;
#if defined(__GNUC__) || defined(__clang__)
            asm volatile("inb %1, %0" : "=a"(v) : "Nd"(_io));
#else
            __asm {
                mov dx, _io
                in al, dx
                mov v, al
            }
#endif
            return v;
        }

        void write(u8 v) const
        {
#if defined(__GNUC__) || defined(__clang__)
            asm volatile("outb %0, %1" ::"a"(v), "Nd"(_io));
#else
            __asm {
                mov al, v
                mov dx, _io
                out dx, al
            }
#endif
        }

        [[nodiscard]] constexpr u16 address() const { return _io; }

    private:
        u16 _io;
    };

} // namespace hw
