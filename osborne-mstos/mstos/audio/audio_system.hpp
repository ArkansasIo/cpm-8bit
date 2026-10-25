// mstos/audio/audio_system.hpp — MSTOS 2.2 system sound event system
//
// Original CPM-8BIT implementation: manifest (TOML subset), .8snd parser,
// event router with priorities + rate limits + bounded queue, retro backend
// driving CPM8-AUDIO8, and a headless test sink.
#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace mstos::audio
{

    // ---------- .8snd sequence (original text format) ----------
    struct Snd8Op
    {
        uint8_t voice;    // 0 or 1
        uint8_t control;  // AUDC low 4 bits (0xFF = unchanged)
        uint8_t freq;     // AUDF low 5 bits (0xFF = unchanged)
        uint8_t vol;      // AUDV low 4 bits (0xFF = unchanged)
        uint32_t wait_ms; // WAIT duration after this op
    };

    struct Snd8Sequence
    {
        std::string event_id;
        std::vector<Snd8Op> ops;
        bool ok = false;
    };

    // parse ".8snd" text; rejects bad syntax safely
    Snd8Sequence parse_snd8(const std::string &text, const std::string &event_id = {});

    // ---------- manifest entry ----------
    struct SoundEvent
    {
        std::string id;          // "sound.boot.post_pass"
        std::string modern_path; // may be empty (retro-only)
        std::string retro_path;  // may be empty (modern-only)
        double volume = 0.35;    // per-event volume, clamped to [0,1]
        int priority = 1;        // 0..4
    };

    struct ManifestLimits
    {
        double master_volume = 1.0;
        size_t queue_capacity = 16;
        uint32_t rate_limit_ms = 250;
    };

    struct SoundManifest
    {
        int version = 0;
        std::string default_theme;
        ManifestLimits limits;
        std::map<std::string, SoundEvent> events;
        bool ok = false;
    };

    // parse the versioned manifest (TOML subset: [section] keys and values)
    SoundManifest parse_manifest(const std::string &toml_text);

    // ---------- backends ----------
    class AudioBackend
    {
    public:
        virtual ~AudioBackend() = default;
        virtual const char *name() const = 0;
        // render an event; returns false if the backend cannot play it
        virtual bool play(const SoundEvent &ev, const Snd8Sequence *retro,
                          std::string &err) = 0;
    };

    // headless sink: validates + hashes the rendered stream (CI)
    class HeadlessSink final : public AudioBackend
    {
    public:
        const char *name() const override { return "headless"; }
        bool play(const SoundEvent &ev, const Snd8Sequence *retro, std::string &err) override;
        uint64_t hash() const { return _hash; }
        size_t samples() const { return _samples; }

    private:
        uint64_t _hash = 0;
        size_t _samples = 0;
    };

    // ---------- event router ----------
    class SystemSoundService
    {
    public:
        static SystemSoundService &instance();

        bool load_manifest_text(const std::string &toml_text, std::string &err);
        void set_backend(AudioBackend *b) { _backend = b; }
        void set_theme(std::string_view theme) { _theme.assign(theme); }
        void set_enabled(bool e) { _enabled = e; }
        [[nodiscard]] bool enabled() const { return _enabled; }
        [[nodiscard]] const std::string &theme() const { return _theme; }

        // applications call this; never throws, never blocks boot
        void play(std::string_view event_id);

        // diagnostics
        [[nodiscard]] size_t dropped_count() const { return _dropped; }
        [[nodiscard]] const SoundManifest &manifest() const { return _manifest; }

    private:
        SystemSoundService() = default;
        SoundManifest _manifest;
        AudioBackend *_backend = nullptr;
        std::string _theme = "retro-8bit";
        bool _enabled = true;
        size_t _dropped = 0;
        uint64_t _last_play_ms[8] = {}; // rate limiting per bucket
    };

} // namespace mstos::audio
