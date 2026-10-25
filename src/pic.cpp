// src/pic.cpp — 8259 PIC driver
#include "pic.hpp"

namespace hw
{

    bool Pic::probe()
    {
        // read interrupt mask register via OCW3 RR (read register command, 8-bit)
        _mcmd.write(0x0A);
        return true;
    }

    void Pic::io_wait() const
    {
        // ISA-era I/O delay via port 0x80 (8-bit write to diagnostic port)
        const Port8 diag(0x80);
        diag.write(0);
    }

    void Pic::init()
    {
        // ICW1: begin init, cascade mode, edge triggered, ICW4 needed
        _mcmd.write(0x11);
        io_wait();
        _scmd.write(0x11);
        io_wait();

        // ICW2: remap vectors (8-bit offset)
        _mdata.write(VECTOR_BASE);
        io_wait();
        _sdata.write(u8(VECTOR_BASE + 8));
        io_wait();

        // ICW3: master has slave on IRQ2; slave is identity 2
        _mdata.write(0x04);
        io_wait();
        _sdata.write(0x02);
        io_wait();

        // ICW4: 8086 mode
        _mdata.write(0x01);
        io_wait();
        _sdata.write(0x01);
        io_wait();

        // mask all except timer(0), keyboard(1), cascade(2)
        mask_irq(0xFF); // helper writes master mask
        unmask_irq(IRQ_TIMER);
        unmask_irq(IRQ_KBD);
        unmask_irq(2);
    }

    void Pic::send_eoi(u8 irq) const
    {
        if (irq >= 8)
            _scmd.write(0x20); // OCW2: non-specific EOI
        _mcmd.write(0x20);
    }

    void Pic::mask_irq(u8 irq) const
    {
        // irq: 0-7 master; write 0xFF once here means "set base mask"
        if (irq == 0xFF)
        {
            _mdata.write(0xFF);
            _sdata.write(0xFF);
            return;
        }
        u8 mask = _mdata.read();
        mask |= u8(1u << irq);
        _mdata.write(mask);
    }

    void Pic::unmask_irq(u8 irq) const
    {
        u8 mask = _mdata.read();
        mask &= u8(~(1u << irq));
        _mdata.write(mask);
    }

} // namespace hw
