// osbex/boot/proto_boot.hpp — full OS Prototype-style boot screen, loader and load screen
//
// Simulated boot chain, Osborne-virtual-hardware true to the configured machine:
//   1. BIOS/POST screen        (already handled by Post)
//   2. OSP Boot Manager-style boot menu    (kernel selection, timeout countdown)
//   3. OS Prototype kernel console    ([    0.000000] style dmesg lines, streaming)
//   4. Loader progress bar     (initramfs unpack, modules)
//   5. Load screen / splash    (spinner dots + percentage → hands off to shell)
#pragma once
#include <string>

namespace osbex::boot
{
    struct KernelChoice
    {
        std::string label;     // menu title
        std::string version;   // e.g. "6.8.0-osbex"
        bool recovery = false; // recovery/rescue initramfs
    };

    // OSP Boot Manager-style menu; returns the chosen kernel (index) or -1 on ESC.
    int boot_menu(const KernelChoice *kernels, int count, int timeout_seconds);

    // Streams dmesg-style kernel lines with realistic timestamps.
    void kernel_console(const KernelChoice &k, int lines = 24);

    // Loader progress: [####    ] 47% bar driven by real simulated work stages.
    void loader_bar(const std::string &stage, int percent, int width = 40);

    // Full initramfs + module load sequence, ends with "switch_root".
    void load_initramfs(const KernelChoice &k);

    // Load screen: spinner + percentage to 100%, then clears for the OS.
    void load_screen(const std::string &title, int total_ticks = 40);

    // Convenience: runs 3–5 in order for the chosen kernel.
    void boot_prototype(const KernelChoice &k);
} // namespace osbex::boot
