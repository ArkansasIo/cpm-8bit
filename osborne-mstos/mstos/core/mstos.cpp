// mstos/core/mstos.cpp — boot chain: Linux → initramfs → MSTOS services
#include "mstos.hpp"
#include "../shell/shell.hpp"
#include "emulator/hardware.hpp"
#include <chrono>
#include <thread>

namespace mstos::core
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

    Mstos &Mstos::instance()
    {
        static Mstos m;
        return m;
    }

    void Mstos::load_linux_kernel()
    {
        // Phase 5: the 64-bit Linux kernel is responsible for scheduling, memory,
        // processes, storage, networking, drivers, security, syscalls.
        // Inside the Windows emulator the kernel image load is simulated; the
        // same stage sequence will drive a real image on hardware (M10).
        auto &disp = emulator::Machine::instance().display();
        disp.write("\nLinux Kernel ...... LOADING\n");
        pause_ms(300);
        disp.write("\rLinux Kernel ...... LOADED\n");
        stage_line("Initramfs");
    }

    void Mstos::load_initramfs() { /* folded into load_linux_kernel staging */ }

    void Mstos::start_services()
    {
        auto &disp = emulator::Machine::instance().display();
        disp.write("MSTOS 2.2 ......... STARTING\n\n");
        // system services: filesystem, device services, CP/M compat, app mgmt
        disp.write("[  OK  ] mstos-fs       (C: MSTOS filesystem)\n");
        disp.write("[  OK  ] mstos-dev      (device services)\n");
        disp.write("[  OK  ] mstos-cpm      (CP/M compatibility)\n");
        disp.write("[  OK  ] mstos-gui      (desktop environment)\n\n");
    }

    void Mstos::run_shell()
    {
        shell::run();
    }

    [[noreturn]] void Mstos::start()
    {
        load_linux_kernel();
        start_services();
        run_shell(); // never returns
    }

} // namespace mstos::core
