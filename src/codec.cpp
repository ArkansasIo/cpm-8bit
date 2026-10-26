// src/codec.cpp — 8-bit byte <-> tone encoder/decoder over the PC speaker
#include "codec.hpp"
#include "audio.hpp"

namespace kernel
{
    void AudioCodec::play_byte(u8 byte)
    {
        Speaker &spk = Speaker::instance();
        // high nibble tone, gap, low nibble tone — 8-bit nibble encoding
        spk.beep(nibble_to_tone(u8(byte >> 4)), TONE_MS);
        spk.beep(nibble_to_tone(u8(byte & 0x0F)), TONE_MS);
        spk.beep(SYNC_HZ, 10); // sync blip between bytes
    }

    void AudioCodec::play_buffer(const u8 *data, u16 len)
    {
        // frame: start marker (two low tones), data, end marker (two high tones)
        Speaker &spk = Speaker::instance();
        spk.beep(200, 60);
        spk.beep(200, 60);
        for (u16 i = 0; i < len; ++i)
            play_byte(data[i]);
        spk.beep(1600, 60);
        spk.beep(2000, 90);
    }

    u8 AudioCodec::tone_to_nibble(u16 freq_hz)
    {
        if (freq_hz < BASE_HZ - STEP_HZ / 2)
            return 0xFF;
        const u16 rel = u16(freq_hz - BASE_HZ + STEP_HZ / 2);
        const u8 nib = u8(rel / STEP_HZ);
        return nib <= 15 ? nib : u8(0xFF);
    }

    // ---- event sounds with distinct encoder signatures ----
    void AudioCodec::encode_loading(const u8 *data, u16 len)
    {
        // loading: hear the actual bytes streaming in (cassette-load style)
        play_buffer(data, len > 8 ? 8 : len); // cap at 8 bytes: keeps boot snappy
    }

    void AudioCodec::encode_saving(const u8 *data, u16 len)
    {
        // saving: start chirp, byte stream, rising "commit" tail
        Speaker &spk = Speaker::instance();
        spk.beep(700, 60);
        play_buffer(data, len > 8 ? 8 : len);
        spk.beep(1050, 90);
    }

    void AudioCodec::encode_reading()
    {
        // reading: triple sync blip = read/verify pass (no data audible)
        Speaker &spk = Speaker::instance();
        spk.beep(SYNC_HZ, 25);
        spk.beep(SYNC_HZ, 25);
        spk.beep(SYNC_HZ, 25);
    }

    void AudioCodec::encode_writing()
    {
        // writing: three-byte header "WRT" encoded as tones
        play_byte('W');
        play_byte('R');
        play_byte('T');
    }
} // namespace kernel
