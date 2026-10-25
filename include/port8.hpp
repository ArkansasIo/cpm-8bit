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
            asm volatile("inb %1, %0" : "=a"(v) : "Nd"(_io));
            return v;
        }

        void write(u8 v) const
        {
            asm volatile("outb %0, %1" ::"a"(v), "Nd"(_io));
        }

        [[nodiscard]] constexpr u16 address() const { return _io; }

    private:
        u16 _io;
    };

} // namespace hw
