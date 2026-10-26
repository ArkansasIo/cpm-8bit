// osbex/core/osbex.cpp — boot chain: OS Prototype → initramfs → osbex services
#include "osbex.hpp"
#include "../shell/shell.hpp"
#include "emulator/hardware.hpp"
#include <chrono>
#include <thread>

namespace osbex::core
{

    static void pause_ms(unsigned ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static void stage_line(const char *what)
    {
        auto &disp = emulator::Machine::instance().display();
        disp.write(std::string(what) + " ...... LOADING\n");
        pause_ms(250);
        // cursor-rewrite to DONE
        disp.write("\r" + std::string(what) + " ...... DONE\n");
    }

    osbex &osbex::instance()
    {
        static osbex m;
        return m;
    }

    void osbex::load_linux_kernel()
    {
        // Phase 5: the 64-bit OS Prototype kernel is responsible for scheduling, memory,
        // processes, storage, networking, drivers, security, syscalls.
        // Inside the Windows emulator the kernel image load is simulated; the
        // same stage sequence will drive a real image on hardware (M10).
        auto &disp = emulator::Machine::instance().display();
        disp.write("\nOS Prototype Kernel ...... LOADING\n");
        pause_ms(300);
        disp.write("\rOS Prototype Kernel ...... LOADED\n");
        stage_line("Initramfs");
    }

    void osbex::load_initramfs() { /* folded into load_linux_kernel staging */ }

    void osbex::start_services()
    {
        auto &disp = emulator::Machine::instance().display();
        disp.write("osbex 2.2 ......... STARTING\n\n");
        // system services: filesystem, device services, CP/M compat, app mgmt
        disp.write("[  OK  ] osbex-fs       (C: osbex filesystem)\n");
        disp.write("[  OK  ] osbex-dev      (device services)\n");
        disp.write("[  OK  ] osbex-cpm      (CP/M compatibility)\n");
        disp.write("[  OK  ] osbex-gui      (desktop environment)\n\n");
    }

    void osbex::run_shell()
    {
        shell::run();
    }

    [[noreturn]] void osbex::start()
    {
        load_linux_kernel();
        start_services();
        run_shell(); // never returns
    }

} // namespace osbex::core
