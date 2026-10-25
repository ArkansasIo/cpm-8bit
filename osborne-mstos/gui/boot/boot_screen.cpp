// gui/boot/boot_screen.cpp — Osborne boot splash (Phase 2 visual)
#include "boot_screen.hpp"
#include "emulator/hardware.hpp"

namespace gui::boot
{

    void show_splash()
    {
        auto &disp = emulator::Machine::instance().display();
        disp.clear();
        disp.write(
            "+==========================================+\n"
            "|        O S B O R N E   1 / MSTOS         |\n"
            "|        character-oriented  80x25         |\n"
            "+==========================================+\n");
    }

} // namespace gui::boot
