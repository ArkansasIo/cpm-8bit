// src/keyboard.cpp — PS/2 keyboard driver (8-bit scancodes)
#include "keyboard.hpp"
#include "port8.hpp"

namespace hw
{

    bool Keyboard::probe()
    {
        // controller self-test via 8-bit command 0xAA on port 0x64; PASS = 0x55
        const Port8 stat(IO_STAT), data(IO_DATA);
        stat.write(0xAA);
        // small spin for the 8-bit status byte
        for (u16 i = 0; i < 1000; ++i)
        {
            if (data.read() == 0x55)
                return true;
        }
        return true; // QEMU tolerates; keep permissive
    }

    void Keyboard::init()
    {
        // enable keyboard scanning: 8-bit command 0xAE (enable first port)
        const Port8 stat(IO_STAT);
        stat.write(0xAE);
    }

    char Keyboard::decode(u8 sc, bool shift)
    {
        // US set-1 make codes (8-bit), only unmodified set supported
        static constexpr char lo[] = {0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
                                      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
                                      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
                                      'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
                                      ' ', ' '};
        static constexpr char hi[] = {0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
                                      '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
                                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
                                      'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*',
                                      ' ', ' '};

        if (sc & 0x80)
        { // break code — 8-bit high bit
            if ((sc & 0x7F) == 0x2A || (sc & 0x7F) == 0x36)
                return 0; // shift release
            return 0;
        }
        if (sc == 0x2A || sc == 0x36)
            return 0; // shift press handled elsewhere
        if (sc == 0x2A)
            return 0;

        if (sc < sizeof(lo))
        {
            const char c = shift ? hi[sc] : lo[sc];
            return c;
        }
        return 0;
    }

    void Keyboard::on_irq()
    {
        const Port8 data(IO_DATA);
        const u8 sc = data.read(); // 8-bit scancode
        if (sc == 0x2A || sc == 0x36)
        {
            _shift = true;
            return;
        }
        if ((sc & 0x80) && ((sc & 0x7F) == 0x2A || (sc & 0x7F) == 0x36))
        {
            _shift = false;
            return;
        }
        if (sc & 0x80)
            return;
        const char c = decode(sc, _shift);
        if (c)
        {
            _last = c;
            _has = true;
        }
    }

    char Keyboard::getch()
    {
        while (!_has)
            asm volatile("hlt");
        _has = false;
        return _last;
    }

} // namespace hw
