// osbex/recovery/recovery.cpp — minimal recovery environment (Phase 9)
// capabilities: hardware diagnostics, boot-log inspection, fsck, kernel
// selection, osbex recovery, config repair, virtual disk inspection
#include "recovery.hpp"
#include "emulator/hardware.hpp"
#include "osbex/boot/post.hpp"

namespace osbex::recovery
{

    static void diagnostics()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        boot::Post post;
        disp.write("\n");
        post.run();
    }

    static void boot_log()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.write("\nBOOT LOG\n--------\n");
        disp.write("(boot log inspection lands with persistent logging — M9)\n");
    }

    static void disk_inspect()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        disp.write("\nVIRTUAL DISK INSPECTION\n");
        disp.write("STORAGE: " + std::to_string(Machine::instance().disk().gigabytes()) + " GB virtual disk\n");
        disp.write("(image sector viewer lands with disk-image pipeline — M9)\n");
    }

    void main_menu()
    {
        using namespace emulator;
        auto &disp = Machine::instance().display();
        auto &kbd = Machine::instance().keyboard();

        disp.write("osbex RECOVERY\n");
        disp.write(" 1. Hardware diagnostics\n 2. Boot log\n 3. Virtual disk inspection\n 4. Exit recovery\n");
        disp.write("Select: ");
        while (true)
        {
            char c;
            bool enter, esc;
            if (!kbd.poll_key(c, enter, esc))
                continue;
            disp.write("\n");
            switch (c)
            {
            case '1':
                diagnostics();
                break;
            case '2':
                boot_log();
                break;
            case '3':
                disk_inspect();
                break;
            default:
                return;
            }
            disp.write("\nSelect: ");
        }
    }

    [[noreturn]] void run()
    {
        main_menu();
        std::exit(0);
    }

} // namespace osbex::recovery

// bridge used by the boot manager on POST failure
void recovery_main() { osbex::recovery::run(); }
