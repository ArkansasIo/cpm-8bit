// osbex/core/osbex.hpp — osbex 2.2 operating environment (Phase 6)
#pragma once
#include <string>

namespace osbex::core
{

    // Boot stages between POST and the osbex environment.
    enum class Stage
    {
        KernelLoad,
        Initramfs,
        osbexStart
    };

    class osbex
    {
    public:
        static osbex &instance();

        // Linux Kernel → Initramfs → osbex 2.2 — never returns
        [[noreturn]] void start();

    private:
        osbex() = default;
        void load_linux_kernel(); // Phase 5 (simulated until image pipeline)
        void load_initramfs();
        void start_services();
        void run_shell(); // Phase 6 shell (Milestone 5–6)
    };

} // namespace osbex::core
