// emulator/main.cpp — Windows application entry (Phase 1)
// Boots the virtual Osborne: config → POST → boot manager → osbex 2.2.
#include "hardware.hpp"
#include "osbex/boot/post.hpp"
#include "osbex/boot/boot_manager.hpp"
#include "osbex/boot/linux_boot.hpp"
#include "osbex/core/osbex.hpp"
#include "osbex/recovery/recovery.hpp"
#include "osbex/compatibility/cpm.hpp"
#include "osbex/audio/audio_system.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace emulator;

// boot-critical rule: audio must never block or fail the boot path
static void init_sound_system() {
    using namespace osbex::audio;
    auto& snd = SystemSoundService::instance();
    static HeadlessSink sink;                 // headless backend (CI-safe)
    snd.set_backend(&sink);
    std::ifstream f("config/audio/system-sounds.toml", std::ios::binary);
    if (f) {
        std::ostringstream ss; ss << f.rdbuf();
        std::string err;
        if (!snd.load_manifest_text(ss.str(), err)) return;   // log only
    }
    snd.play("sound.boot.power_on");
}

static HardwareConfig parse_args(int argc, char **argv)
{
    HardwareConfig c;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--ram") == 0 && i + 1 < argc)
            c.ram_mb = std::strtoul(argv[++i], nullptr, 10);
        else if (std::strcmp(argv[i], "--disk") == 0 && i + 1 < argc)
            c.disk_gb = std::strtoul(argv[++i], nullptr, 10);
        else if (std::strcmp(argv[i], "--nonet") == 0)
            c.network = false;
        else if (std::strcmp(argv[i], "--state") == 0 && i + 1 < argc)
            c.state_file = argv[++i];
    }
    return c;
}

int main(int argc, char **argv)
{
    init_sound_system();
    Machine::instance().configure(parse_args(argc, argv));

    // Phase 3: POST — diagnostics run against the configured virtual hardware.
    osbex::boot::Post post;
    if (!post.run())
    {
        osbex::audio::SystemSoundService::instance().play("sound.boot.post_fail");
        // failed required component: prevent normal boot, drop to recovery
        osbex::boot::BootManager::recovery_after_post_failure(post);
        return 1;
    }

    // Phase 4: boot manager
    osbex::boot::BootManager mgr;
    const auto choice = mgr.run();
    switch (choice)
    {
    case osbex::boot::BootChoice::osbex:
        break; // continue below
    case osbex::boot::BootChoice::Recovery:
        osbex::recovery::run();
        return 0;
    case osbex::boot::BootChoice::Diagnostics:
        post.run_extended();
        return 0;
    case osbex::boot::BootChoice::CpmDisk:
        osbex::compat::boot_cpm();
        return 0;
    case osbex::boot::BootChoice::Exit:
        return 0;
    }

    // Phase 5/6: GRUB menu → Linux kernel console → loader → load screen → OSBEX 2.2
    using osbex::boot::KernelChoice;
    const KernelChoice kernels[] = {
        {"OSBEX 2.2 (default)", "6.8.0-osbex", false},
        {"OSBEX 2.2 (recovery)", "6.8.0-osbex", true},
    };
    const int pick = osbex::boot::boot_menu(kernels, 2, 5);
    if (pick < 0)
        return 0; // ESC in the boot menu = power off
    osbex::boot::boot_linux(kernels[pick]);

    osbex::core::osbex::instance().start();
    return 0;
}
