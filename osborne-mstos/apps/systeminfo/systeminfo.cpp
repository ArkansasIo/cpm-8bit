// apps/systeminfo/systeminfo.cpp — System Information app
#include "systeminfo.hpp"
#include "emulator/hardware.hpp"

namespace apps::systeminfo
{

    void launch()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.clear();
        disp.write("+----------------------------------------+\n");
        disp.write("|           SYSTEM INFORMATION           |\n");
        disp.write("+----------------------------------------+\n");
        disp.write(" Machine : Osborne 1 (virtual)\n");
        disp.write(" OS      : MSTOS 2.2\n");
        disp.write(" Platform: 64-bit Linux (simulated under Windows)\n");
        disp.write(" CPU     : " + Machine::instance().cpu().model() + "\n");
        disp.write("           " + std::to_string(Machine::instance().cpu().cores()) + " cores\n");
        disp.write(" Memory  : " + std::to_string(Machine::instance().ram().megabytes()) + " MB\n");
        disp.write(" Storage : " + std::to_string(Machine::instance().disk().gigabytes()) + " GB\n");
        disp.write(" Video   : 80x25 text, 640x480 graphics\n");
        disp.write(" Network : " + std::string(Machine::instance().net().available() ? "available" : "disabled") + "\n");
        disp.write("+----------------------------------------+\n");
    }

} // namespace apps::systeminfo
