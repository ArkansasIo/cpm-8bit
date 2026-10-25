// include/device.hpp — abstract hardware device base (OOP polymorphism)
#pragma once
#include "types.hpp"
#include "port8.hpp"

namespace hw
{

    class Device
    {
    public:
        constexpr Device(const char *name, u16 io_base) : _name(name), _io(io_base) {}
        virtual ~Device() = default;

        virtual bool probe() = 0; // detect hardware
        virtual void init() = 0;  // bring up
        virtual const char *name() const { return _name; }
        [[nodiscard]] u16 io_base() const { return _io; }

    private:
        const char *_name;
        u16 _io;
    };

} // namespace hw
