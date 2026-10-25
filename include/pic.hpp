// include/pic.hpp — 8259 Programmable Interrupt Controller (8-bit cascaded PIC)
#pragma once
#include "device.hpp"

namespace hw
{

    class Pic final : public Device
    {
    public:
        static constexpr u16 MASTER_CMD = 0x20, MASTER_DATA = 0x21;
        static constexpr u16 SLAVE_CMD = 0xA0, SLAVE_DATA = 0xA1;
        static constexpr u8 IRQ_TIMER = 0, IRQ_KBD = 1;
        static constexpr u8 VECTOR_BASE = 0x20; // remap IRQs 0-15 -> int 0x20-0x2F

        Pic() : Device("i8259-pic", MASTER_CMD) {}

        bool probe() override;
        void init() override;

        void send_eoi(u8 irq) const;
        void mask_irq(u8 irq) const;
        void unmask_irq(u8 irq) const;
        void io_wait() const;

    private:
        const Port8 _mcmd{MASTER_CMD}, _mdata{MASTER_DATA};
        const Port8 _scmd{SLAVE_CMD}, _sdata{SLAVE_DATA};
    };

} // namespace hw
