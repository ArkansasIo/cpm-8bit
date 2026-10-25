// include/vga.hpp — 8-bit VGA text-mode driver
#pragma once
#include "device.hpp"

namespace hw
{

    class Vga final : public Device
    {
    public:
        static constexpr u16 IO_CRTC_INDEX = 0x3D4;
        static constexpr u16 IO_CRTC_DATA = 0x3D5;
        static constexpr u16 MEM_TEXT = 0xB800;
        static constexpr u8 COLS = 80, ROWS = 25;

        // 8-bit color attribute (foreground/background packed in one byte)
        enum Color : u8
        {
            Black = 0,
            Blue,
            Green,
            Cyan,
            Red,
            Magenta,
            Brown,
            LightGray,
            DarkGray,
            LightBlue,
            LightGreen,
            LightCyan,
            LightRed,
            Pink,
            Yellow,
            White
        };

        Vga() : Device("vga-text", IO_CRTC_INDEX) {}

        bool probe() override { return true; } // always present on this era
        void init() override;

        void clear(u8 attr = make_attr(White, Black));
        void put_char(char c, u8 attr, u8 x, u8 y);
        void write(const char *s, u8 attr);
        void scroll();
        void update_cursor();

        [[nodiscard]] constexpr u8 cursor_x() const { return _x; }
        [[nodiscard]] constexpr u8 cursor_y() const { return _y; }
        void set_attr(u8 attr) { _attr = attr; }

        static constexpr u8 make_attr(u8 fg, u8 bg) { return u8((bg << 4) | (fg & 0x0F)); }

    private:
        u8 _x = 0, _y = 0;
        u8 _attr = make_attr(White, Black);

        [[nodiscard]] static u16 *buffer() { return reinterpret_cast<u16 *>(MEM_TEXT * 16u); }
    };

} // namespace hw
