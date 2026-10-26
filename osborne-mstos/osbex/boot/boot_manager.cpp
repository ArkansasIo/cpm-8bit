// osbex/boot/boot_manager.cpp — Osborne Boot Manager (Phase 4)
#include "boot_manager.hpp"
#include "post.hpp"
#include "emulator/hardware.hpp"
#include <chrono>
#include <thread>

namespace osbex::boot
{
    static void pause_ms(unsigned ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    static constexpr const char *kItems[] = {
        "1. Boot osbex 2.2",
        "2. Recovery Mode",
        "3. Diagnostics",
        "4. CP/M Disk",
        "5. Boot Device Selection",
    };

    void BootManager::render(int sel) const
    {
        auto &disp = emulator::Machine::instance().display();
        disp.clear();
        disp.write(title());
        disp.write("\n\n");
        for (int i = 0; i < 5; ++i)
            disp.write(std::string(i == sel ? "> " : "  ") + kItems[i] + "\n");
        disp.write("\nESC - Return   ENTER - Select\n");
    }

    BootChoice BootManager::run()
    {
        auto &kbd = emulator::Machine::instance().keyboard();
        auto &disp = emulator::Machine::instance().display();

        int sel = 0;
        bool need_render = true;
        while (true)
        {
            if (need_render) { render(sel); need_render = false; }
            char c;
            bool enter, esc;
            if (!kbd.poll_key(c, enter, esc))
            {
                pause_ms(16); // wait quietly — no busy redraw
                continue;
            }
            need_render = true;
            if (esc)
                return BootChoice::Exit;
            if (enter)
            {
                switch (sel)
                {
                case 0:
                    return BootChoice::osbex;
                case 1:
                    return BootChoice::Recovery;
                case 2:
                    return BootChoice::Diagnostics;
                case 3:
                    return BootChoice::CpmDisk;
                case 4: // boot device selection: list virtual disks
                    disp.clear();
                    disp.write("BOOT DEVICE SELECTION\n\n");
                    disp.write("  A: Osborne disk image\n  B: Osborne disk image\n  C: osbex filesystem\n  D: Modern storage\n\nPress any key...");
                    {
                        char c2;
                        bool e1, e2;
                        while (!kbd.poll_key(c2, e1, e2))
                        {
                            pause_ms(16); // wait quietly
                        }
                    }
                    break;
                }
                sel = sel == 4 ? sel : sel; // stay
                if (sel == 4)
                    continue;
            }
            // simple up/down via 'w'/'s' and number keys
            if (c == 'w' || c == 'W')
                sel = (sel + 4) % 5;
            if (c == 's' || c == 'S')
                sel = (sel + 1) % 5;
            if (c >= '1' && c <= '5')
                return BootChoice(int(c - '1'));
        }
    }

    void BootManager::recovery_after_post_failure(Post &post)
    {
        auto &disp = emulator::Machine::instance().display();
        disp.write("\nEntering recovery environment...\n");
        for (const auto &r : post.results())
            if (!r.ok)
                disp.write(std::string("  FAILED: ") + r.component + " — " + r.detail + "\n");
        // Phase 9 recovery environment
        extern void recovery_main();
        recovery_main();
    }

} // namespace osbex::boot
