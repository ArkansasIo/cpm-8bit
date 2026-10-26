// src/kernel.cpp — kernel object: init sequence, IRQ plumbing, tiny shell
#include "kernel.hpp"
#include "audio.hpp"
#include "codec.hpp"
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
        vga.write("OS Prototype 3.0.0 — redesigned C++23 kernel / MS-DOS 2.2 API / 8-bit hardware\n",
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
        // audio subsystem comes up last so its first sound means "OS alive"
        Speaker::instance().init();
        Speaker::instance().sound_boot();
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
                vga.write("OS Prototype 3.0.0 kernel 0.1 — C++23 — DOS 2.2 API layer\n",
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
            else if (line[0] == 's' && line[1] == 'o' && line[2] == 'u' && line[3] == 'n' && line[4] == 'd')
            {
                // SOUND <name> — play an event sound: boot loading read write save ok error
                Speaker &spk = Speaker::instance();
                if (line[6] == 'b') spk.sound_boot();
                else if (line[6] == 'l') spk.sound_loading();
                else if (line[6] == 'r') { spk.sound_read(); AudioCodec::encode_reading(); }
                else if (line[6] == 'w') { spk.sound_write(); AudioCodec::encode_writing(); }
                else if (line[6] == 's')
                {
                    // demo: encode a sample byte stream so the save "speaks" data
                    static const u8 sample[4] = {0x0F, 0x42, 0x2A, 0x7F};
                    spk.sound_save();
                    AudioCodec::encode_saving(sample, 4);
                }
                else if (line[6] == 'o') spk.sound_ok();
                else if (line[6] == 'e') spk.sound_error();
                else vga.write("sound: boot loading read write save ok error\n",
                               hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
            }
            else if (line[0] == 't' && line[1] == 'o' && line[2] == 'n' && line[3] == 'e')
            {
                // TONE <byte-hex> — codec demo: encode byte to tones, decode back
                u8 byte = 0xA7;
                // parse two hex digits if given
                if (len >= 8)
                {
                    const u8 hi = line[5] <= '9' ? u8(line[5] - '0') : u8(line[5] & 0x0F);
                    const u8 lo = line[7] <= '9' ? u8(line[7] - '0') : u8(line[7] & 0x0F);
                    byte = u8((hi << 4) | lo);
                }
                AudioCodec::play_byte(byte);
                // decode both nibble tones back and display
                char b[6];
                vga.write("encoded 0x", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(byte, b), hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(" as tones ", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(AudioCodec::nibble_to_tone(u8(byte >> 4)), b),
                          hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write("/", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(u32_to_dec(AudioCodec::nibble_to_tone(u8(byte & 0x0F)), b),
                          hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
                vga.write(" Hz\n", hw::Vga::make_attr(hw::Vga::LightGray, hw::Vga::Black));
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
                vga.write("VER TIME TICK ECHO SUB SOUND TONE HELP EXIT EXEC\n",
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

    // early marker: raw-asm write of 'A' to VGA (segment-safe, no C++ pointers)
    asm volatile("movw $0xB800, %%ax\n\tmovw %%ax, %%es\n\tmovw $0x0F41, %%ax\n\tmovw %%ax, %%es:(0)" ::: "ax", "memory");
    Bus &bus = Bus::instance();
    // marker E: kmain entered (ES-safe, FS segment)
    asm volatile("movw $0xB800, %%ax\n\tmovw %%ax, %%fs\n\tmovw $0x0F45, %%ax\n\tmovw %%ax, %%fs:(1000)" ::: "ax", "memory");
    bus.register_device(bus.vga());
    bus.register_device(bus.pic());
    bus.register_device(bus.pit());
    bus.register_device(bus.keyboard());

    bus.vga().init();
    // marker F: survived vga.init (clear wipes earlier markers)
    asm volatile("movw $0x0F46, %%ax\n\tmovw %%ax, %%fs:(1200)" ::: "ax", "memory");
    bus.probe_all(); // probe devices (OOP virtual dispatch)
    bus.init_all();  // init: PIC remap, PIT 100Hz, keyboard
    asm volatile("movw $0x0F44, %%ax\n\tmovw %%ax, %%fs:(480)" ::: "ax", "memory"); // 'D'

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
