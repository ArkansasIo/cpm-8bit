// emulator/main.cpp — Windows application entry (Phase 1)
// Boots the virtual Osborne: config → POST → boot manager → MSTOS 2.2.
#include "hardware.hpp"
#include "mstos/boot/post.hpp"
#include "mstos/boot/boot_manager.hpp"
#include "mstos/core/mstos.hpp"
#include "mstos/recovery/recovery.hpp"
#include "mstos/compatibility/cpm.hpp"
#include "mstos/audio/audio_system.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace emulator;

// boot-critical rule: audio must never block or fail the boot path
static void init_sound_system() {
    using namespace mstos::audio;
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
    mstos::boot::Post post;
    if (!post.run())
    {
        mstos::audio::SystemSoundService::instance().play("sound.boot.post_fail");
        // failed required component: prevent normal boot, drop to recovery
        mstos::boot::BootManager::recovery_after_post_failure(post);
        return 1;
    }

    // Phase 4: boot manager
    mstos::boot::BootManager mgr;
    const auto choice = mgr.run();
    switch (choice)
    {
    case mstos::boot::BootChoice::Mstos:
        break; // continue below
    case mstos::boot::BootChoice::Recovery:
        mstos::recovery::run();
        return 0;
    case mstos::boot::BootChoice::Diagnostics:
        post.run_extended();
        return 0;
    case mstos::boot::BootChoice::CpmDisk:
        mstos::compat::boot_cpm();
        return 0;
    case mstos::boot::BootChoice::Exit:
        return 0;
    }

    // Phase 5/6: Linux kernel → initramfs → MSTOS 2.2
    mstos::core::Mstos::instance().start();
    return 0;
}
