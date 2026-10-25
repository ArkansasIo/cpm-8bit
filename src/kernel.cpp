// src/kernel.cpp — kernel object: init sequence, IRQ plumbing, tiny shell
#include "kernel.hpp"
#include "bus.hpp"
#include "port8.hpp"

// MinGW decorates C symbols with a leading underscore; the asm defines
// _irq_stub/_dos_api_gate, which appear here as irq_stub/dos_api_gate.
extern "C" char irq_stub[];
extern "C" char dos_api_gate[];

namespace kernel
{

    void Kernel::banner()
    {
        auto &vga = _bus.vga();
        vga.write("TinyOS-8 — C++23 kernel / MS-DOS 2.2 API / 8-bit hardware\n",
                  hw::Vga::make_attr(hw::Vga::Yellow, hw::Vga::Black));
        vga.write("devices probed:\n", hw::Vga::make_attr(hw::Vga::LightCyan, hw::Vga::Black));
        for (u8 i = 0; i < _bus.device_count(); ++i)
        {
            vga.write("  - ", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            vga.write(_bus.vga().name(), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
        }
        vga.write("INT 21h DOS API ready. Type commands at the prompt.\n\n",
                  hw::Vga::make_attr(hw::Vga::LightGreen, hw::Vga::Black));
    }

    void Kernel::remap_interrupts()
    {
        // install IRQ stub and DOS gate vectors (segment 0, offsets below)
        // note: flat tiny model, so vectors point at 0:offset.
        // The IVT lives at physical 0x0000; use a volatile pointer and
        // suppress array-bounds diagnostics for the deliberate 0-based write.
        volatile u16 *ivt = reinterpret_cast<volatile u16 *>(0x0000);
        const u32 stub  = reinterpret_cast<u32>(irq_stub);
        const u32 gate  = reinterpret_cast<u32>(dos_api_gate);
        ivt[0x20 * 2]     = u16(stub & 0xFFFF);
        ivt[0x20 * 2 + 1] = 0;
        ivt[0x21 * 2]     = u16(gate & 0xFFFF);
        ivt[0x21 * 2 + 1] = 0;
    }

    // simple itoa into a fixed buffer (8-bit era style)
    static char *u32_to_dec(u32 v, char *out)
    {
        char tmp[11];
        u8 i = 0;
        if (!v)
            tmp[i++] = '0';
        while (v)
        {
            tmp[i++] = char('0' + v % 10);
            v /= 10;
        }
        u8 j = 0;
        while (i)
            out[j++] = tmp[--i];
        out[j] = 0;
        return out;
    }

    void Kernel::shell()
    {
        auto &vga = _bus.vga();
        char line[64];
        u8 len = 0;

        while (true)
        {
            vga.write("A:\\> ", hw::Vga::make_attr(hw::Vga::LightGreen, hw::Vga::Black));
            len = 0;
            while (true)
            {
                const char c = _dos.read_char();
                if (c == '\b')
                {
                    if (len > 0)
                    {
                        --len;
                        vga.write("\b \b", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                    }
                    continue;
                }
                if (c == '\r' || c == '\n')
                {
                    vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                    break;
                }
                if (c >= 32 && len < sizeof(line) - 1)
                {
                    line[len++] = c;
                    char s[2] = {c, 0};
                    vga.write(s, hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                }
            }
            line[len] = 0;
            if (len == 0)
                continue;

            if (line[0] == 'v' && line[1] == 'e' && line[2] == 'r')
            {
                vga.write("TinyOS-8 kernel 0.1 — C++23 — DOS 2.2 API layer\n",
                          hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 't' && line[1] == 'i' && line[2] == 'm' && line[3] == 'e')
            {
                u8 h, m, s;
                char b[4];
                _dos.get_time(h, m, s);
                vga.write("uptime h=", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(h, b), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(" m=", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(m, b), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(" s=", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(s, b), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 't' && line[1] == 'i' && line[2] == 'c' && line[3] == 'k')
            {
                char b[12];
                vga.write("PIT ticks: ", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(_bus.pit().ticks(), b), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 'e' && line[1] == 'c' && line[2] == 'h' && line[3] == 'o')
            {
                // MS-DOS 2.2 internal command: ECHO <text>
                for (u8 i = 5; i < len; ++i)
                {
                    char s[2] = {line[i], 0};
                    vga.write(s, hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                }
                vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 'e' && line[1] == 'x' && line[2] == 'i' && line[3] == 't')
            {
                // invoke DOS 2.2 terminate via int 21h AH=4Ch (8-bit AL = exit code)
                u8 code = 0;
                asm volatile("movb $0x4C, %%ah; movb $0, %%al; int $0x21" ::: "ah", "al");
                (void)code;
                vga.write("exit(0) via INT 21h AH=4Ch\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 'e' && line[1] == 'x' && line[2] == 'e' && line[3] == 'c')
            {
                // EXEC the demo program via INT 21h AH=01h sample (see programs/HELLO.ASM)
                vga.write("load programs/hello.bin at 0x2000 and far call... (see Makefile)\n",
                          hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else
            {
                vga.write("Bad command or file name\n",
                          hw::Vga::make_attr(hw::Vga::LightRed, hw::Vga::Black));
            }
        }
    }

} // namespace kernel

// ---------------- kernel entry (called from interrupts.asm) ----------------
extern "C" void kmain()
{
    using namespace kernel;
    using namespace hw;

    Bus &bus = Bus::instance();
    bus.register_device(bus.vga());
    bus.register_device(bus.pic());
    bus.register_device(bus.pit());
    bus.register_device(bus.keyboard());

    bus.vga().init();
    bus.probe_all(); // probe devices (OOP virtual dispatch)
    bus.init_all();  // init: PIC remap, PIT 100Hz, keyboard

    Kernel::instance().boot(); // banner → interrupts → shell, never returns
}

namespace kernel
{
    [[noreturn]] void Kernel::boot()
    {
        banner();
        remap_interrupts();
        shell(); // never returns
        for (;;) asm volatile("hlt"); // unreachable, satisfies noreturn
    }
}
