// src/audio.cpp — PC speaker implementation (8-bit hardware paths only)
#include "audio.hpp"
#include "bus.hpp"
#include "port8.hpp"

namespace kernel
{
    // 8253 timer channels/ports + speaker gate — all 8-bit port I/O
    static constexpr u16 PIT_CMD = 0x43;
    static constexpr u16 PIT_CH2 = 0x42;
    static constexpr u16 SPKR_PORT = 0x61; // gate + wave select bits
    static constexpr u32 PIT_BASE_HZ = 1193182;

    // busy-wait delay driven by the PIT tick counter (no BIOS calls)
    void Speaker::delay_ms(u16 ms)
    {
        // PIT ticks at 100 Hz (configured by hw::Pit), so 1 tick = 10 ms;
        // Pit lives on the Bus (singleton of singletons).
        const u32 start = hw::Bus::instance().pit().ticks();
        while ((hw::Bus::instance().pit().ticks() - start) < u32(ms / 10 + 1))
            asm volatile("hlt");
    }

    void Speaker::set_freq(u16 freq_hz)
    {
        if (freq_hz == 0)
            return;
        // 8-bit era math: divider = 1193182 / freq, written low byte then high
        u32 div = PIT_BASE_HZ / freq_hz;
        if (div > 0xFFFF)
            div = 0xFFFF;
        const hw::Port8 ch2(PIT_CH2), cmd(PIT_CMD);
        cmd.write(0xB6);      // ch2, mode 3, binary
        ch2.write(u8(div & 0xFF));
        ch2.write(u8((div >> 8) & 0xFF));
    }

    void Speaker::gate_on()
    {
        const hw::Port8 spkr(SPKR_PORT);
        spkr.write(u8(spkr.read() | 0x03)); // gate + waveform enable
    }

    void Speaker::gate_off()
    {
        const hw::Port8 spkr(SPKR_PORT);
        spkr.write(u8(spkr.read() & ~0x03));
    }

    void Speaker::init()
    {
        gate_off();
        _init = true;
    }

    void Speaker::beep(u16 freq_hz, u16 ms)
    {
        if (!_init)
            init();
        set_freq(freq_hz);
        gate_on();
        delay_ms(ms);
        gate_off();
    }

    void Speaker::silence() { gate_off(); }

    // ---- printer-style event signatures ----
    void Speaker::sound_boot()
    {
        // rising two-tone fanfare: A4 -> E5
        beep(440, 90);
        beep(659, 140);
    }

    void Speaker::sound_loading()
    {
        beep(880, 25); // short progress tick, like a print head pass
    }

    void Speaker::sound_read()
    {
        beep(1200, 35); // high blip = data in
    }

    void Speaker::sound_write()
    {
        beep(500, 45); // low blip = data out
    }

    void Speaker::sound_save()
    {
        // rising chirp pair = "saved"
        beep(700, 60);
        beep(1050, 90);
    }

    void Speaker::sound_ok()
    {
        // major third (C5+E5) = success
        beep(523, 70);
        beep(659, 70);
    }

    void Speaker::sound_error()
    {
        // harsh buzz: two low square pulses
        beep(160, 180);
        beep(160, 180);
    }

    void Speaker::sound_keyclick()
    {
        beep(2000, 4); // 4 ms tick
    }
} // namespace kernel
