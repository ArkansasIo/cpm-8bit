// gui/desktop/desktop.cpp — desktop shell: launcher, terminal, file manager,
// settings, system info — Osborne-era typography at modern resolutions.
// Milestone 7; currently renders as a character-mode desktop over the
// virtual display so the architecture exists before a graphics compositor.
#include "desktop.hpp"
#include "emulator/hardware.hpp"
#include "mstos/shell/shell.hpp"

namespace gui::desktop
{

    static void render(int sel)
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.clear();
        disp.write("+----------------------------------------+\n");
        disp.write("|  MSTOS 2.2 DESKTOP                     |\n");
        disp.write("+----------------------------------------+\n");
        static constexpr const char *items[] = {
            " Terminal",
            " File Manager",
            " Settings",
            " System Info",
            " Shut Down",
        };
        for (int i = 0; i < 5; ++i)
            disp.write(std::string(i == sel ? "| >" : "|  ") + items[i] + std::string(37, ' ') + "|\n");
        disp.write("+----------------------------------------+\n");
        disp.write("  UP/DOWN (w/s) move, ENTER select\n");
    }

    [[noreturn]] void run()
    {
        using namespace emulator;
        auto &kbd = Machine::instance().keyboard();

        int sel = 0;
        while (true)
        {
            render(sel);
            char c;
            bool enter, esc;
            if (!kbd.poll_key(c, enter, esc))
                continue;
            if (c == 'w' || c == 'W')
                sel = (sel + 4) % 5;
            if (c == 's' || c == 'S')
                sel = (sel + 1) % 5;
            if (enter)
            {
                switch (sel)
                {
                case 0:
                    mstos::shell::run();
                    break; // never returns (terminal = shell)
                case 1:
                    Machine::instance().display().write("File manager: Milestone 7 (pending)\n");
                    break;
                case 2:
                    Machine::instance().display().write("Settings: Milestone 7 (pending)\n");
                    break;
                case 3:
                    Machine::instance().display().write("System info: see SYSINFO in shell\n");
                    break;
                case 4:
                    std::exit(0);
                }
            }
        }
    }

} // namespace gui::desktop
