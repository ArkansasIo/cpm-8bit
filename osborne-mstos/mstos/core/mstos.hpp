// mstos/core/mstos.hpp — MSTOS 2.2 operating environment (Phase 6)
#pragma once
#include <string>

namespace mstos::core
{

    // Boot stages between POST and the MSTOS environment.
    enum class Stage
    {
        KernelLoad,
        Initramfs,
        MstosStart
    };

    class Mstos
    {
    public:
        static Mstos &instance();

        // Linux Kernel → Initramfs → MSTOS 2.2 — never returns
        [[noreturn]] void start();

    private:
        Mstos() = default;
        void load_linux_kernel(); // Phase 5 (simulated until image pipeline)
        void load_initramfs();
        void start_services();
        void run_shell(); // Phase 6 shell (Milestone 5–6)
    };

} // namespace mstos::core
