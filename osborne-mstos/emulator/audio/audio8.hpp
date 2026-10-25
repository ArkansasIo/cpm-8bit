// emulator/audio8.hpp — CPM8-AUDIO8 virtual device (TIA-inspired, original)
//
// Two-voice programmable sound generator. Deterministic: fixed-point phase
// accumulation, integer math only. Final MMIO addresses are versioned in
// IO_MAP.md; the register contract follows docs/AUDIO_SPEC.md.
#pragma once
#include "../hardware.hpp"
#include <cstdint>
#include <array>

namespace emulator::audio
{

    class Audio8 final : public VirtualDevice
    {
    public:
        static constexpr uint32_t BASE_AUDIO_CLOCK = 31400; // guest audio clock Hz
        static constexpr uint32_t DEFAULT_RATE = 44100;     // host sample rate

        // register offsets within the device (MMIO base assigned in IO_MAP.md)
        enum Reg : uint8_t
        {
            AUDC0,
            AUDF0,
            AUDV0,
            AUDC1,
            AUDF1,
            AUDV1,
            AUCTRL,
            AUDIO_STATUS,
            REG_COUNT
        };

        Audio8() : VirtualDevice(), _cfg_ref(_default_cfg()) {}

        const char *name() const override { return "AUDIO"; }

        // POST self test: generate a known buffer and hash it (deterministic)
        bool self_test(std::string &detail) override;

        void reset();
        void write_reg(Reg r, uint8_t v); // guest register write
        [[nodiscard]] uint8_t read_reg(Reg r) const;

        // generate n samples at `rate` into out (deterministic, mono)
        void generate(uint32_t rate, int16_t *out, size_t n);

        // pause/resume + snapshot (M16/SNAPSHOT_SPEC)
        void set_paused(bool p) { _paused = p; }
        bool save_state(std::string &blob) const;
        bool load_state(const std::string &blob);

        [[nodiscard]] bool enabled() const { return (_regs[AUCTRL] & 1) != 0; }

    private:
        struct Voice
        {
            uint8_t control = 0;      // AUDC low 4 bits
            uint8_t divider = 0;      // AUDF low 5 bits
            uint8_t volume = 0;       // AUDV low 4 bits
            uint32_t phase = 0;       // fixed-point phase (Q32.32)
            uint32_t noise = 0x7C4Du; // LFSR state (original)
        };

        static HardwareConfig &_default_cfg()
        {
            static HardwareConfig c;
            return c;
        }

        int16_t render_voice(Voice &v, uint32_t rate);

        std::array<uint8_t, REG_COUNT> _regs{};
        std::array<Voice, 2> _voice{};
        bool _paused = false;
        const HardwareConfig &_cfg_ref;
    };

} // namespace emulator::audio
