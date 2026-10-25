// include/kernel.hpp — the kernel object (Linux-kernel style init sequence)
#pragma once
#include "types.hpp"
#include "bus.hpp"
#include "dos_api.hpp"

namespace kernel
{

    class Kernel
    {
    public:
        static Kernel &instance()
        {
            static Kernel k;
            return k;
        }

        // never returns; runs the kernel forever
        [[noreturn]] void run();

        // one-shot boot entry: banner → interrupt setup → shell
        [[noreturn]] void boot();

        [[nodiscard]] u32 uptime_ticks() const { return _bus.pit().ticks(); }

    private:
        Kernel() = default;

        void banner();           // print boot banner + device table
        void remap_interrupts(); // install ISR/dos gate vectors
        void shell();            // tiny DOS-like command shell (COMMAND.COM vibes)

        hw::Bus &_bus = hw::Bus::instance();
        dos::DosApi &_dos = dos::DosApi::instance();
    };

} // namespace kernel
