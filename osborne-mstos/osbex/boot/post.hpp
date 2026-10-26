// osbex/boot/post.hpp — Power-On Self Test over the virtual Osborne hardware
#pragma once
#include <string>
#include <vector>

namespace emulator
{
    class VirtualDevice;
}

namespace osbex::boot
{

    struct PostResult
    {
        const char *component;
        bool ok;
        std::string detail;
    };

    class Post
    {
    public:
        // run all checks; false when a REQUIRED component failed (blocks boot)
        bool run();
        // extended diagnostics (boot manager option 3)
        void run_extended();

        const std::vector<PostResult> &results() const { return _results; }

    private:
        bool check(emulator::VirtualDevice &dev, bool required);
        std::vector<PostResult> _results;
        bool _fatal = false;
    };

} // namespace osbex::boot
