// src/dos_api.cpp — MS-DOS 2.2 API implementation over the 8-bit drivers
#include "dos_api.hpp"

namespace dos
{

    // ---------------- FileHandle (FCB) ----------------
    bool FileHandle::parse_name(const char *name11)
    {
        // DOS 2.2 packed FCB name: 8 chars + 3 chars, space padded
        for (u8 i = 0; i < 8; ++i)
            _fcb.name[i] = name11[i];
        for (u8 i = 0; i < 3; ++i)
            _fcb.ext[i] = name11[8 + i];
        _fcb.drive = 0;
        return true;
    }

    void FileHandle::open()
    {
        _open = true;
        _fcb.record_size = 128; // DOS 2.2 default record size
        _fcb.file_size = 0;
        _fcb.date = u16((2026 - 1980) << 9); // packed DOS date
    }

    void FileHandle::close() { _open = false; }

    // ---------------- DosApi ----------------
    void DosApi::put_char(char c)
    {
        if (!_vga)
            return;
        if (c == '\b')
        {
            u8 x = _vga->cursor_x();
            if (x > 0)
            {
                _vga->put_char(' ', _vga->make_attr(hw::Vga::White, hw::Vga::Black), u8(x - 1), _vga->cursor_y());
            }
            // simplest: just reprint cursor position handled by VGA internals
            return;
        }
        const u16 cell = u16(u8(c)) | (u16(_vga->make_attr(hw::Vga::LightGray, hw::Vga::Black)) << 8);
        (void)cell; // single-char path goes through write
        char s[2] = {c, 0};
        _vga->write(s, _vga->make_attr(hw::Vga::LightGray, hw::Vga::Black));
    }

    void DosApi::write_str(const char *s)
    {
        if (_vga)
            _vga->write(s, _vga->make_attr(hw::Vga::LightGray, hw::Vga::Black));
    }

    char DosApi::read_char()
    {
        if (_kbd)
            return _kbd->getch();
        return 0;
    }

    void DosApi::get_time(u8 &h, u8 &m, u8 &s)
    {
        if (!_pit)
            return;
        const u32 t = _pit->ticks(); // ticks since boot at PIT hz
        const u32 total = t / _pit->hz();
        s = u8(total % 60);
        m = u8((total / 60) % 60);
        h = u8((total / 3600) % 24);
    }

    void DosApi::exit(u8 code)
    {
        _exit_code = code;
        _exited = true;
        write_str("\nProcess terminated.");
    }

    void DosApi::set_int_vector(u8 vec, u16 seg, u16 off)
    {
        _vec_seg[vec] = seg;
        _vec_off[vec] = off;
        // program the real-mode IVT at 0x0000:vec*4 (4 bytes, 8-bit halves)
        u16 *ivt = reinterpret_cast<u16 *>(0x0000);
        ivt[vec * 2] = off;
        ivt[vec * 2 + 1] = seg;
    }

    void DosApi::get_int_vector(u8 vec, u16 &seg, u16 &off)
    {
        off = _vec_off[vec];
        seg = _vec_seg[vec];
    }

    // Dispatch: AH=function, DL/DH/AL are 8-bit args
    void DosApi::dispatch(u8 ah, u8 al, u8 dl, u8 dh, u8 &ret_al, UNUSED u8 &ret_ah)
    {
        switch (static_cast<Fn>(ah))
        {
        case Fn::ReadCharEcho:
        {
            const char c = read_char();
            put_char(c); // echo
            ret_al = u8(c);
            break;
        }
        case Fn::WriteChar:
            put_char(char(dl));
            break;
        case Fn::WriteString:
            // real DOS uses DS:DX; here the router passes nothing — kernel strings
            write_str("?");
            break;
        case Fn::BufferedInput:
            // simplified: read one line, echo
            while (true)
            {
                const char c = read_char();
                put_char(c);
                if (c == '\n' || c == '\r')
                    break;
            }
            break;
        case Fn::SetIntVector:
            set_int_vector(al, u16(u16(dh) << 8 | dl), 0);
            break;
        case Fn::GetTime:
            get_time(dl, dh, ret_al);
            break;
        case Fn::GetIntVector:
            ret_al = u8(_vec_off[al] & 0xFF);
            break;
        case Fn::Terminate:
            exit(al);
            break;
        default:
            write_str("\nDOS: unsupported function AH=");
            // print AH as two hex nibbles using 8-bit math
            const char hexd[] = "0123456789ABCDEF";
            put_char(hexd[(ah >> 4) & 0xF]);
            put_char(hexd[ah & 0xF]);
            break;
        }
    }

} // namespace dos

// ---------------- C ABI interrupt handlers ----------------

// IRQ dispatcher: master PIC vectors start at 0x20
extern "C" void hw_irq_handler()
{
    using namespace hw;
    Bus &bus = Bus::instance();
    // NOTE: vectors remapped to 0x20..0x2F by Pic::init
    // read the ISR-served IRQ from OCW3 register (8-bit poll)
    u8 irq = 0;
    asm volatile("movb $0x0B, %%al; outb %%al, $0x20; inb $0x20, %%al" : "=a"(irq)::);
    const u8 line = u8(irq & 0x07);
    if (line == Pic::IRQ_TIMER)
    {
        bus.pit().tick();
    }
    else if (line == Pic::IRQ_KBD)
    {
        bus.keyboard().on_irq();
    }
    bus.pic().send_eoi(line);
}

// INT 21h router: pulls 8-bit registers off the stack frame saved by stub
extern "C" void dos_int21_handler()
{
    // registers were pushed by stub; here we read via a minimal trampoline
    u8 ah = 0, al = 0, dl = 0, dh = 0, ra = 0, rh = 0;
    asm volatile(
        "movb %%ah, %0 \n movb %%al, %1 \n movb %%dl, %2 \n movb %%dh, %3"
        : "=r"(ah), "=r"(al), "=r"(dl), "=r"(dh));
    dos::DosApi::instance().dispatch(ah, al, dl, dh, ra, rh);
    asm volatile("movb %0, %%al" ::"r"(ra));
}
