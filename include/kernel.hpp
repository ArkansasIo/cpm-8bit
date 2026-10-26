// include/kernel.hpp — redesigned kernel object
//
// Redesign pillars (Linux-inspired, C++23 OOP):
//   1. Subsystem objects with a lifecycle: init() -> start() -> status()
//      (Ksched = scheduler, Kvfs = VFS + ramfs, Kdos = DOS 2.2 API gate,
//       Kcon  = console/shell). Each is registered into the kernel's
//      subsystem table and brought up in dependency order — like Linux's
//      initcalls, but objects.
//   2. Tasks + cooperative round-robin scheduler (sched.hpp)
//   3. VFS layer separating DOS file semantics from device drivers (vfs.hpp)
#pragma once
#include "types.hpp"
#include "bus.hpp"
#include "dos_api.hpp"
#include "sched.hpp"
#include "vfs.hpp"

namespace kernel
{
    // a kernel subsystem: uniform lifecycle, uniform status
    class Subsystem
    {
    public:
        virtual ~Subsystem() = default;
        virtual const char *name() const = 0;
        virtual bool init() = 0;   // bring-up; false = boot must fail/panic
        virtual void start() {}    // after all inits succeeded
        virtual void status(char *out, u8 cap) = 0; // one-line report
    };

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

        // one-shot boot entry: subsystems -> banner -> interrupts -> shell
        [[noreturn]] void boot();

        [[nodiscard]] u32 uptime_ticks() const { return _bus.pit().ticks(); }
        [[nodiscard]] Scheduler &scheduler() { return _sched; }
        [[nodiscard]] Vfs &vfs() { return _vfs; }
        [[nodiscard]] bool all_subsystems_ok() const { return _init_ok; }

    private:
        Kernel() = default;

        void register_subsystems();
        bool init_subsystems();  // returns false on any failure
        void start_subsystems();
        void banner();           // print boot banner + subsystem status
        void remap_interrupts(); // install ISR/dos gate vectors
        [[noreturn]] void shell(); // tiny DOS-like command shell

        hw::Bus &_bus = hw::Bus::instance();
        dos::DosApi &_dos = dos::DosApi::instance();
        Scheduler &_sched = Scheduler::instance();
        Vfs &_vfs = Vfs::instance();

        Subsystem *_subs[8]{};
        u8 _sub_count = 0;
        bool _init_ok = false;
    };

} // namespace kernel
