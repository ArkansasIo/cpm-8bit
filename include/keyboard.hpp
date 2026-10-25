// include/keyboard.hpp — PS/2 keyboard driver (8-bit scancode protocol)
#pragma once
#include "device.hpp"

namespace hw
{

    class Keyboard final : public Device
    {
    public:
        static constexpr u16 IO_DATA = 0x60; // 8-bit data port
        static constexpr u16 IO_STAT = 0x64; // status register

        Keyboard() : Device("ps2-keyboard", IO_DATA) {}

        bool probe() override;
        void init() override;

        void on_irq(); // called from IRQ1
        [[nodiscard]] char last_char() const { return _last; }
        [[nodiscard]] bool has_key() const { return _has; }
        char getch(); // blocking; clears has_key

    private:
        char _last = 0;
        bool _has = false;
        bool _shift = false;

        static char decode(u8 sc, bool shift);
    };

} // namespace hw
