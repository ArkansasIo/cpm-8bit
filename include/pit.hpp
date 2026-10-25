// include/pit.hpp — 8253/8254 Programmable Interval Timer (8-bit data bus)
#pragma once
#include "device.hpp"

namespace hw
{

    class Pit final : public Device
    {
    public:
        static constexpr u16 IO_CH0 = 0x40;     // channel 0 data port (8-bit writes)
        static constexpr u16 IO_CMD = 0x43;     // mode/command register
        static constexpr u32 BASE_HZ = 1193182; // 1.193182 MHz crystal

        Pit() : Device("i8253-pit", IO_CH0) {}

        bool probe() override { return true; }
        void init() override;

        void set_frequency(u32 hz); // program channel 0 square wave

        void tick(); // called from timer IRQ
        [[nodiscard]] u32 ticks() const { return _ticks; }
        [[nodiscard]] u32 hz() const { return _hz; }

    private:
        u32 _ticks = 0;
        u32 _hz = 100;
    };

} // namespace hw
