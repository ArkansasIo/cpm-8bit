// src/vga.cpp — VGA text mode driver
#include "vga.hpp"

namespace hw
{

    void Vga::init()
    {
        clear();
        _x = _y = 0;
    }

    void Vga::clear(u8 attr)
    {
        _attr = attr;
        const u16 cell = u16(u16(' ') | (u16(attr) << 8));
        for (u16 i = 0; i < COLS * ROWS; ++i)
            buffer()[i] = cell;
        _x = _y = 0;
        update_cursor();
    }

    void Vga::put_char(char c, u8 attr, u8 x, u8 y)
    {
        if (x >= COLS || y >= ROWS)
            return;
        buffer()[u16(y) * COLS + x] = u16(u8(c)) | (u16(attr) << 8);
    }

    void Vga::scroll()
    {
        for (u8 y = 1; y < ROWS; ++y)
            for (u8 x = 0; x < COLS; ++x)
                buffer()[u16(y - 1) * COLS + x] = buffer()[u16(y) * COLS + x];
        // clear last line
        const u16 cell = u16(u16(' ') | (u16(_attr) << 8));
        for (u8 x = 0; x < COLS; ++x)
            buffer()[u16(ROWS - 1) * COLS + x] = cell;
    }

    void Vga::write(const char *s, u8 attr)
    {
        for (const char *p = s; *p; ++p)
        {
            const char c = *p;
            if (c == '\n')
            {
                _x = 0;
                ++_y;
            }
            else if (c == '\r')
            {
                _x = 0;
            }
            else
            {
                put_char(c, attr, _x, _y);
                if (++_x >= COLS)
                {
                    _x = 0;
                    ++_y;
                }
            }
            if (_y >= ROWS)
            {
                scroll();
                _y = ROWS - 1;
            }
            update_cursor();
        }
    }

    void Vga::update_cursor()
    {
        const u16 pos = u16(_y) * COLS + _x;
        const Port8 idx(IO_CRTC_INDEX), dat(IO_CRTC_DATA);
        idx.write(0x0F); // cursor low register (8-bit)
        dat.write(u8(pos & 0xFF));
        idx.write(0x0E); // cursor high register (8-bit)
        dat.write(u8(pos >> 8));
    }

} // namespace hw
