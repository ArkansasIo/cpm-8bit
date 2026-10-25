// src/pit.cpp — 8253 PIT driver
#include "pit.hpp"
#include "port8.hpp"

namespace hw
{

    void Pit::init()
    {
        set_frequency(100); // 100 Hz default
    }

    void Pit::set_frequency(u32 hz)
    {
        if (hz == 0)
            hz = 1;
        _hz = hz;
        const u16 divisor = u16(BASE_HZ / hz);

        const Port8 ch0(IO_CH0), cmd(IO_CMD);
        // 8-bit command byte: channel 0, lo/hi byte access, mode 3 (square wave), binary
        cmd.write(0x36);
        // 8-bit data writes: low byte then high byte
        ch0.write(u8(divisor & 0xFF));
        ch0.write(u8(divisor >> 8));
    }

    void Pit::tick() { ++_ticks; }

} // namespace hw
