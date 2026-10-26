// osbex/boot/boot_manager.hpp — Osborne Boot Manager (Phase 4)
#pragma once

namespace osbex::boot
{

    enum class BootChoice
    {
        osbex,
        Recovery,
        Diagnostics,
        CpmDisk,
        Exit
    };

    class BootManager
    {
    public:
        BootChoice run();                                          // interactive menu
        static void recovery_after_post_failure(class Post &post); // drop to recovery

    private:
        void render(int selected) const;
        static const char *title() { return "OSBORNE BOOT MANAGER"; }
    };

} // namespace osbex::boot
