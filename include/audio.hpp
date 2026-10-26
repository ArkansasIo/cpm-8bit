// include/audio.hpp — 8-bit PC-speaker audio subsystem
//
// Printer-era sound design: the whole OS is voiced by the 8253-driven
// PC speaker (8-bit data paths only — port writes, no DMA, no mixing).
// Each OS event has its own "signature" tone sequence, like a dot-matrix
// printer's beep codes, so you can *hear* what the OS is doing.
//
// Event sounds:
//   boot      - rising two-tone fanfare      (boot start)
//   loading   - short tick every stage       (loader progress)
//   read      - single high blip             (disk read)
//   write     - low blip                     (disk write)
//   save      - rising chirp pair            (state saved)
//   ok        - pleasant major third         (op success)
//   error     - harsh low buzz               (failure)
//   keyclick  - 2 ms tick                    (shell key press)
#pragma once
#include "types.hpp"

namespace kernel
{
    class Speaker
    {
    public:
        static Speaker &instance()
        {
            static Speaker s;
            return s;
        }

        void init();                    // 8253 ch2 setup, speaker gate off
        void beep(u16 freq_hz, u16 ms); // core primitive (8-bit divider math)
        void silence();

        // ---- OS event sounds (printer-style signatures) ----
        void sound_boot();     // rising fanfare
        void sound_loading();  // progress tick
        void sound_read();     // high blip
        void sound_write();    // low blip
        void sound_save();     // rising chirp pair
        void sound_ok();       // major third
        void sound_error();    // harsh buzz
        void sound_keyclick(); // subtle tick

    private:
        Speaker() = default;
        static void delay_ms(u16 ms);
        static void set_freq(u16 freq_hz); // 8-bit style PIT divider calc
        static void gate_on();
        static void gate_off();
        bool _init = false;
    };
} // namespace kernel
