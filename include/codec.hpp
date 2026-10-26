// include/codec.hpp — 8-bit data encoder/decoder + audio feedback
//
// The OS encodes 8-bit bytes into tone sequences (encoder) and decodes
// tone sequences back into bytes (decoder). Every disk read/write/save
// plays the *actual encoded data* through the PC speaker — you literally
// hear the bytes going in and out, like a tape deck.
//
// Encoding: each byte -> two nibble-tones (high nibble, low nibble),
// mapped onto a 16-step pentatonic scale (240 Hz .. 1440 Hz), with a
// 1800 Hz sync blip between bytes and 120 Hz silence gaps. Decoder maps
// tones back to nibbles.
#pragma once
#include "types.hpp"

namespace kernel
{
    class AudioCodec
    {
    public:
        static constexpr u16 SYNC_HZ = 1800; // inter-byte sync blip
        static constexpr u16 GAP_MS = 14;    // silence gap between tones
        static constexpr u16 TONE_MS = 26;   // nibble tone duration

        static constexpr u8 NIBBLE_TONES = 16; // 4-bit -> 16 tones
        static constexpr u16 BASE_HZ = 240;    // lowest tone
        static constexpr u16 STEP_HZ = 80;     // tone step (pentatonic-ish ladder)

        // play one byte as two tones (high nibble, then low nibble)
        static void play_byte(u8 byte);

        // play a buffer with start/end framing sounds (the "data line")
        static void play_buffer(const u8 *data, u16 len);

        // ---- decode side: used by shell 'tone' command for round-trip demo ----
        // tone frequency -> nibble (nearest ladder step), 0xFF if out of range
        static u8 tone_to_nibble(u16 freq_hz);

        // frequency of a nibble (public for tests/demo)
        static u16 nibble_to_tone(u8 nibble)
        {
            return u16(BASE_HZ + (nibble & 0x0F) * STEP_HZ);
        }

        // event sounds routed through the codec, so each has a distinct
        // encoder signature (loading=long stream, save=short chirp, etc.)
        static void encode_loading(const u8 *data, u16 len); // full data stream
        static void encode_saving(const u8 *data, u16 len);  // framing + full stream
        static void encode_reading();                        // sync blips only (verify pass)
        static void encode_writing();                        // 3-byte header beep
    };
} // namespace kernel
