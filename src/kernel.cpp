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
        vga.write("TinyOS-8 — redesigned C++23 kernel / MS-DOS 2.2 API / 8-bit hardware\n",
                  hw::Vga::make_attr(hw::Vga::Yellow, hw::Vga::Black));
        vga.write("subsystems:\n", hw::Vga::make_attr(hw::Vga::LightCyan, hw::Vga::Black));
        char st[80];
        for (u8 i = 0; i < _sub_count; ++i)
        {
            st[0] = 0;
            _subs[i]->status(st, sizeof st);
            vga.write("  - ", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            vga.write(st, hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
        }
        vga.write("INT 21h DOS API ready. Type HELP for commands.\n\n",
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

    // ---------------- redesigned: subsystem objects ----------------

    // Ksched — cooperative scheduler subsystem
    class Ksched final : public Subsystem
    {
    public:
        const char *name() const override { return "sched"; }
        bool init() override { return true; }
        void start() override { Scheduler::instance().reap(); }
        void status(char *out, u8 cap) override
        {
            char n[4];
            u8 i = 0;
            u8 v = Scheduler::instance().count();
            if (!v) n[i++] = '0';
            while (v) { n[i++] = char('0' + v % 10); v /= 10; }
            u8 j = 0;
            out[j++] = 's'; out[j++] = 'c'; out[j++] = 'h'; out[j++] = 'e'; out[j++] = 'd';
            out[j++] = ':'; out[j++] = ' ';
            while (i) out[j++] = n[--i];
            const char *tail = " task(s), round-robin";
            for (const char *p = tail; *p && j < cap - 1; ++p) out[j++] = *p;
            out[j] = 0;
        }
    };

    // Kvfs — VFS + ramfs subsystem
    class Kvfs final : public Subsystem
    {
    public:
        const char *name() const override { return "vfs"; }
        bool init() override
        {
            return Vfs::instance().mount(_ramfs, 0);
        }
        void status(char *out, u8 cap) override
        {
            u8 j = 0;
            const char *s = "vfs: ramfs mounted (16K)";
            for (const char *p = s; *p && j < cap - 1; ++p) out[j++] = *p;
            out[j] = 0;
        }

    private:
        RamFs _ramfs;
    };

    // Kdos — DOS 2.2 API gate subsystem
    class Kdos final : public Subsystem
    {
    public:
        const char *name() const override { return "dos"; }
        bool init() override { return true; }
        void status(char *out, u8 cap) override
        {
            u8 j = 0;
            const char *s = "dos: INT 21h gate armed";
            for (const char *p = s; *p && j < cap - 1; ++p) out[j++] = *p;
            out[j] = 0;
        }
    };

    void Kernel::register_subsystems()
    {
        static Ksched ksched;
        static Kvfs kvfs;
        static Kdos kdos;
        _subs[_sub_count++] = &ksched;
        _subs[_sub_count++] = &kvfs;
        _subs[_sub_count++] = &kdos;
    }

    bool Kernel::init_subsystems()
    {
        for (u8 i = 0; i < _sub_count; ++i)
            if (!_subs[i]->init())
                return false;
        _init_ok = true;
        return true;
    }

    void Kernel::start_subsystems()
    {
        for (u8 i = 0; i < _sub_count; ++i)
            _subs[i]->start();
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
            else if (line[0] == 's' && line[1] == 'u' && line[2] == 'b')
            {
                // SUB — report subsystem status (redesigned kernel)
                char st[80];
                for (u8 i = 0; i < _sub_count; ++i)
                {
                    st[0] = 0;
                    _subs[i]->status(st, sizeof st);
                    vga.write("  ", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                    vga.write(st, hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                    vga.write("\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                }
            }
            else if (line[0] == 'h' && line[1] == 'e' && line[2] == 'l' && line[3] == 'p')
            {
                vga.write("VER TIME TICK ECHO SUB HELP EXIT EXEC\n",
                          hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
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

    // early marker: prove we reached kmain
    { volatile u16 *v = reinterpret_cast<volatile u16 *>(0xB8000); v[0] = 0x014B; v[1] = 0x014D; } // 'K','M'
    Bus &bus = Bus::instance();
    bus.register_device(bus.vga());
    bus.register_device(bus.pic());
    bus.register_device(bus.pit());
    bus.register_device(bus.keyboard());

    bus.vga().init();
    bus.probe_all(); // probe devices (OOP virtual dispatch)
    bus.init_all();  // init: PIC remap, PIT 100Hz, keyboard

    Kernel::instance().boot(); // subsystems -> banner -> interrupts -> shell
}

namespace kernel
{
    [[noreturn]] void Kernel::boot()
    {
        register_subsystems();
        if (!init_subsystems())
        {
            // kernel panic path (16-bit analogue of Linux's panic())
            _bus.vga().write("PANIC: subsystem init failed\n",
                             hw::Vga::make_attr(hw::Vga::Red, hw::Vga::Black));
            for (;;) asm volatile("hlt");
        }
        banner();
        remap_interrupts();
        start_subsystems();
        shell(); // never returns
        for (;;) asm volatile("hlt"); // unreachable, satisfies noreturn
    }
}
