// osbex/compatibility/cpm.cpp — CP/M/Z80 compatibility layer (Phase 8, isolated
// from osbex core and OS Prototype; runs legacy software against virtual drives)
#include "cpm.hpp"
#include "emulator/hardware.hpp"
#include <filesystem>
#include <fstream>
#include <map>

namespace osbex::compat
{

    namespace
    {
        struct DriveState
        {
            std::string image; // attached image file, empty if none
            bool mounted = false;
        };
        std::map<Drive, DriveState> g_drives{
            {Drive::A, {}}, {Drive::B, {}}, {Drive::C, {}}, {Drive::D, {}}};

        std::string upper(std::string s)
        {
            for (char &c : s)
                if (c >= 'a' && c <= 'z')
                    c = char(c - 32);
            return s;
        }
    } // namespace

    void command([[maybe_unused]] const std::string &raw)
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.write("CP/M compatibility layer 2.2\n");
        disp.write("  A: " + g_drives[Drive::A].image + (g_drives[Drive::A].mounted ? " [mounted]" : "") + "\n");
        disp.write("  B: " + g_drives[Drive::B].image + (g_drives[Drive::B].mounted ? " [mounted]" : "") + "\n");
        disp.write("  C: osbex filesystem\n");
        disp.write("  D: modern storage\n");
        disp.write("usage: CPM MOUNT <drive> <image.dsk> | CPM DIR <drive> | CPM RUN <prog.com>\n");
    }

    void boot_cpm()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.clear();
        disp.write("CP/M DISK BOOT\n\n");
        const auto &a = g_drives[Drive::A];
        if (!a.mounted)
        {
            disp.write("No disk image mounted on A:.\n");
            disp.write("Mount one from the osbex shell: CPM MOUNT A: mydisk.dsk\n");
            return;
        }
        disp.write("Booting " + a.image + " ...\n(Z80 emulation pipeline lands in Milestone 8)\n");
    }

} // namespace osbex::compat
