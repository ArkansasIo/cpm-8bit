// include/bus.hpp — device registry over polymorphic Device*
#pragma once
#include "device.hpp"
#include "vga.hpp"
#include "pic.hpp"
#include "pit.hpp"
#include "keyboard.hpp"

namespace hw
{

    // Polymorphic device bus — owns concrete drivers behind Device& references.
    class Bus
    {
    public:
        static Bus &instance()
        {
            static Bus b;
            return b;
        }

        void register_device(Device &d) { _devices[_count++] = &d; }

        void probe_all()
        {
            for (u8 i = 0; i < _count; ++i)
                _devices[i]->probe();
        }
        void init_all()
        {
            for (u8 i = 0; i < _count; ++i)
                _devices[i]->init();
        }
        [[nodiscard]] u8 device_count() const { return _count; }

        // typed accessors (kernel owns the singletons)
        Vga &vga() { return _vga; }
        Pic &pic() { return _pic; }
        Pit &pit() { return _pit; }
        Keyboard &keyboard() { return _kbd; }

    private:
        Bus() = default;
        Device *_devices[8]{};
        u8 _count = 0;
        Vga _vga;
        Pic _pic;
        Pit _pit;
        Keyboard _kbd;
    };

} // namespace hw
