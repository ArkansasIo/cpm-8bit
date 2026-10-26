// osbex/boot/proto_boot.cpp — OS Prototype-style boot chain over the virtual Osborne
#include "proto_boot.hpp"
#include "emulator/hardware.hpp"
#include <chrono>
#include <cstdio>
#include <thread>

namespace osbex::boot
{
    using emulator::Machine;
    using Clock = std::chrono::steady_clock;

    // ---- performance model: boot pacing scales with the virtual machine ----
    // On 8GB+ virtual RAM the host can retire the whole boot chain quickly,
    // so all artificial delays are scaled down (down to ~8% of nominal) and
    // the kernel console lines stream in larger batches. Goal: boot faster
    // than Windows/OS Prototype/macOS cold starts on equivalent hardware.
    struct BootProfile { int delay_scale; int console_batch; bool fast_grub; };
    static BootProfile boot_profile()
    {
        const uint32_t mb = Machine::instance().ram().megabytes();
        if (mb >= 8192)  return {8, 6, true};   // 8GB+: fastest
        if (mb >= 4096)  return {25, 3, false}; // 4GB+: fast
        if (mb >= 2048)  return {50, 2, false}; // 2GB: moderate
        return {100, 1, false};                 // <2GB: nominal (era-authentic)
    }
    static int scaled(int ms)
    {
        return ms * boot_profile().delay_scale / 100;
    }

    static void pause_ms(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(scaled(ms))); }

    static std::string dmesg_ts(double seconds)
    {
        char buf[32];
        std::snprintf(buf, sizeof buf, "[%13.6f]", seconds);
        return buf;
    }

    int boot_menu(const KernelChoice *kernels, int count, int timeout_seconds)
    {
        auto &disp = Machine::instance().display();
        auto &kbd = Machine::instance().keyboard();

        // defensive: no choices / bad args -> behave as "boot the default"
        if (kernels == nullptr || count <= 0)
            return 0;

        disp.write("\n");
        disp.write("  OSP BOOT  version 2.12-osbex\n");
        disp.write("  +----------------------------------------------------------------------------+\n");
        for (int i = 0; i < count; ++i)
            disp.write(std::string("  ") + (i == 0 ? "*" : " ") + " OSBEX 2.2, kernel " + kernels[i].version +
                       (kernels[i].recovery ? " (recovery mode)" : "") + "\n");
        disp.write("  +----------------------------------------------------------------------------+\n");
        disp.write("   Use the up/down keys to select, ENTER to boot, ESC to cancel.\n");

        // countdown line driven by real elapsed time
        auto deadline = Clock::now() + std::chrono::seconds(timeout_seconds);
        int selected = 0;
        while (true)
        {
            const auto remain = std::chrono::duration_cast<std::chrono::seconds>(deadline - Clock::now()).count();
            if (remain < 0)
                break; // timeout -> default entry
            disp.write("\r   The highlighted entry will be executed automatically in " +
                       std::to_string(remain) + "s.   ");
            char c;
            bool enter, esc;
            if (kbd.poll_key(c, enter, esc))
            {
                disp.write("\n");
                if (esc)
                    return -1;
                if (c == 'j' || c == 'J' || c == 's' || c == 'S' || c == 0x42) // down / arrow
                {
                    selected = (selected + 1) % count;
                    disp.write("  (selection moved)\n");
                }
                if (c == 'k' || c == 'K' || c == 'w' || c == 'W' || c == 0x48) // up / arrow
                {
                    selected = (selected + count - 1) % count;
                    disp.write("  (selection moved)\n");
                }
                if (enter)
                    return selected;
                if (c >= '1' && c <= '0' + count)
                    return c - '1';
                if (c == 'e' || c == 'E' || c == 'c' || c == 'C')
                {
                    // OSP Boot Manager-style: edit/commandline not supported in the virtual
                    // machine — acknowledge instead of treating as a normal char
                    disp.write("  (command not available in virtual OSP Boot Manager)\n");
                }
                deadline = Clock::now() + std::chrono::seconds(timeout_seconds);
            }
            else
                pause_ms(50);
        }
        disp.write("\n");
        return selected;
    }

    void kernel_console(const KernelChoice &k, int lines)
    {
        auto &disp = Machine::instance().display();
        auto &m = Machine::instance();
        disp.write("\n[    0.000000] OS Prototype version " + k.version + " (osbex@osborne) (gcc 16.2.0) #1 SMP PREEMPT_DYNAMIC\n");
        disp.write("[    0.000000] Command line: BOOT_IMAGE=/vmlinuz-osbex root=/dev/c0 quiet splash\n");

        struct Line
        {
            double t;
            std::string text;
        };
        const Line all[] = {
            {0.000043, "BIOS-provided physical RAM map (virtual, " + std::to_string(m.ram().megabytes()) + " MB)"},
            {0.000158, "CPU: " + m.cpu().model()},
            {0.000311, "smpboot: " + std::to_string(m.cpu().cores()) + " cores detected"},
            {0.002046, "Memory: " + std::to_string(uint64_t(m.ram().megabytes()) * 1024) + "K available"},
            {0.010233, "virtio_blk virtio0: [vda] " + std::to_string(m.disk().gigabytes()) + "GB virtual disk attached"},
            {0.020987, "Console: colour dummy device 80x25"},
            {0.051271, "clocksource: tsc (virtual PIT 8253 compatible) detected"},
            {0.102584, "PCI: Probing PCI hardware (virtual bus)"},
            {0.133902, "vga16fb: 80x25 text + " + std::to_string(m.config().gfx_w) + "x" + std::to_string(m.config().gfx_h) + " graphics initialized"},
            {0.201177, "i8042: PS/2 keyboard controller (virtual) ready"},
            {0.288450, "8259 PIC: cascaded, IRQ 0-15 remapped"},
            {0.301662, "ext4 filesystem being mounted on /dev/c0: clean"},
            {0.388719, "virtio_net: virtual NIC up (UDP stack OK)"},
            {0.451033, "random: crng init done"},
            {0.512904, "Freeing unused kernel memory: 2048K"},
            {0.600517, "Run /sbin/init as init process"},
        };
        int shown = 0;
        const int batch = boot_profile().console_batch;
        for (const Line &L : all)
        {
            if (shown >= lines)
                break;
            // batch lines without per-line delay on fast machines
            for (int b = 0; b < batch && shown < lines; ++b, ++shown)
                disp.write(dmesg_ts(all[shown].t) + " " + all[shown].text + "\n");
            pause_ms(60);
        }
        if (k.recovery)
            disp.write(dmesg_ts(0.71) + " recovery: initramfs (rescue) selected by user\n");
    }

    void loader_bar(const std::string &stage, int percent, int width)
    {
        auto &disp = Machine::instance().display();
        if (width <= 0)
            width = 40;
        percent = percent < 0 ? 0 : percent > 100 ? 100
                                                  : percent;
        const int filled = width * percent / 100;
        std::string bar = "[" + std::string(size_t(filled), '#') + std::string(size_t(width - filled), ' ') + "]";
        disp.write("\r " + bar + " " + std::to_string(percent) + "%  " + stage + "        ");
    }

    void load_initramfs(const KernelChoice &k)
    {
        auto &disp = Machine::instance().display();
        struct Stage
        {
            std::string name;
            int pct;
            int ms;
        };
        const Stage stages[] = {
            {"Unpacking initramfs" + std::string(k.recovery ? " (rescue)" : ""), 15, 220},
            {"Mounting virtual root filesystem", 35, 260},
            {"Loading kernel modules (virtio, fbcon, snd)", 55, 320},
            {"Starting udev (virtual device enumeration)", 72, 240},
            {"Activating swap (virtual)", 84, 160},
            {"switch_root: moving /dev, /proc, /sys", 96, 200},
            {"init: osbex 2.2 userland", 100, 180},
        };
        disp.write("\n");
        for (const Stage &s : stages)
        {
            loader_bar(s.name, 0);
            pause_ms(s.ms / 4);
            loader_bar(s.name, s.pct / 2);
            pause_ms(s.ms / 4);
            loader_bar(s.name, s.pct);
            pause_ms(s.ms / 2);
            disp.write("\n");
        }
    }

    void load_screen(const std::string &title, int total_ticks)
    {
        auto &disp = Machine::instance().display();
        if (total_ticks <= 0)
            total_ticks = 1; // avoid divide-by-zero below
        static constexpr const char spinner[] = "|/-\\";
        for (int i = 0; i <= total_ticks; ++i)
        {
            const int pct = i * 100 / total_ticks;
            disp.write("\r  " + std::string(1, spinner[i % 4]) + " " + title + "  " + std::to_string(pct) + "%   ");
            pause_ms(45);
        }
        disp.write("\n\n");
        disp.write("  Welcome to OSBEX 2.2\n\n");
        // 8GB+ fast profile: skip the welcome splash pause — shell is the goal
        const bool fast = boot_profile().fast_grub;
        if (!fast)
            pause_ms(400);
    }

    void boot_prototype(const KernelChoice &k)
    {
        kernel_console(k);
        load_initramfs(k);
        load_screen(k.recovery ? "Starting OSBEX 2.2 (recovery)" : "Starting OSBEX 2.2");
    }
} // namespace osbex::boot
