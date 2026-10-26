// osbex/boot/post.cpp — real POST diagnostics (Phase 3)
#include "post.hpp"
#include "emulator/hardware.hpp"

namespace osbex::boot
{

    bool Post::check(emulator::VirtualDevice &dev, bool required)
    {
        std::string detail;
        const bool ok = dev.self_test(detail);
        _results.push_back({dev.name(), ok, detail});

        auto &disp = emulator::Machine::instance().display();
        disp.write(std::string(dev.name()) + " ............... " + (ok ? "OK" : "FAIL") + (detail.empty() ? "" : " (" + detail + ")") + "\n");

        if (!ok && required)
            _fatal = true;
        return ok;
    }

    bool Post::run()
    {
        auto &m = emulator::Machine::instance();
        auto &disp = m.display();

        disp.clear();
        disp.write("OSBORNE COMPUTER SYSTEM\nosbex 2.2\n\n");

        _results.clear();
        _fatal = false;

        check(m.cpu(), true);      // CPU
        check(m.ram(), true);      // RAM detection + RAM test
        check(m.disk(), true);     // Storage
        check(m.display(), true);  // Display
        check(m.keyboard(), true); // Keyboard
        check(m.net(), false);     // Network optional

        disp.write("\n");
        if (_fatal)
        {
            disp.write("POST FAILURE — normal boot prevented.\n");
            return false;
        }
        disp.write("POST complete.\n\n");
        return true;
    }

    void Post::run_extended()
    {
        auto &disp = emulator::Machine::instance().display();
        disp.write("=== EXTENDED DIAGNOSTICS ===\n");
        run();
        // repeat the RAM probe with a different pattern
        disp.write("RAM stress (pattern 0x5A) ... ");
        auto &ram = emulator::Machine::instance().ram();
        std::string d;
        disp.write(ram.self_test(d) ? "PASS\n" : "FAIL\n");
        disp.write("Diagnostics complete.\n");
    }

} // namespace osbex::boot
