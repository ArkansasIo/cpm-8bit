// osbex/audio/audio_system.cpp — implementation (original, deterministic)
#include "audio_system.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace osbex::audio
{

    // ---------------- .8snd parser ----------------
    static bool hex2(const char *s, uint8_t &out)
    {
        if (!std::isxdigit((unsigned char)s[0]) || !std::isxdigit((unsigned char)s[1]))
            return false;
        const auto v = [](char c) -> int
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            return std::toupper((unsigned char)c) - 'A' + 10;
        };
        out = uint8_t((v(s[0]) << 4) | v(s[1]));
        return true;
    }

    Snd8Sequence parse_snd8(const std::string &text, const std::string &event_id)
    {
        Snd8Sequence seq;
        seq.event_id = event_id;
        std::istringstream in(text);
        std::string line;
        bool header = false, end = false;

        while (std::getline(in, line))
        {
            // strip comments / CR
            if (const auto h = line.find('#'); h != std::string::npos)
                line.erase(h);
            while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
                line.pop_back();
            if (line.empty())
                continue;

            std::istringstream words(line);
            std::string tok;
            words >> tok;

            if (tok == "8SND")
            {
                int ver = 0;
                words >> ver;
                header = (ver == 1);
                continue;
            }
            if (tok == "END")
            {
                end = true;
                break;
            }
            if (tok == "WAIT")
            {
                unsigned ms = 0;
                words >> ms; // decimal or zero-padded
                if (!seq.ops.empty())
                    seq.ops.back().wait_ms += ms;
                else
                    seq.ops.push_back({0, 0xFF, 0xFF, 0xFF, ms});
                continue;
            }
            if (tok == "VOICE")
            {
                unsigned v = 9, control = 0xFF, freq = 0xFF, vol = 0xFF;
                words >> v;
                std::string key, val;
                while (words >> key >> val)
                {
                    uint8_t byte = 0;
                    if (val.size() == 2 && hex2(val.c_str(), byte))
                    {
                        if (key == "CONTROL")
                            control = byte;
                        else if (key == "FREQ")
                            freq = byte;
                        else if (key == "VOL")
                            vol = byte;
                    }
                }
                if (v > 1)
                    continue; // invalid voice → skip safely
                seq.ops.push_back({uint8_t(v), uint8_t(control), uint8_t(freq), uint8_t(vol), 0u});
            }
        }

        seq.ok = header && end;
        return seq;
    }

    // ---------------- manifest parser (TOML subset) ----------------
    static std::string strip_quotes(std::string s)
    {
        if (s.size() >= 2 && s.front() == '"')
            s = s.substr(1, s.size() - 2);
        return s;
    }

    SoundManifest parse_manifest(const std::string &toml_text)
    {
        SoundManifest m;
        std::string current_event;

        std::istringstream in(toml_text);
        std::string line;
        while (std::getline(in, line))
        {
            if (const auto h = line.find('#'); h != std::string::npos)
                line.erase(h);
            while (!line.empty() && (line.back() == ' ' || line.back() == '\r'))
                line.pop_back();
            if (line.empty())
                continue;

            if (line.front() == '[')
            {
                const auto close = line.find(']');
                if (close == std::string::npos)
                    continue;
                const std::string section = line.substr(1, close - 1);
                // strip quotes inside [sounds."x"]
                current_event.clear();
                if (section.rfind("sounds.", 0) == 0)
                    current_event = strip_quotes(section.substr(7));
                else if (section == "theme" || section == "limits")
                    current_event.clear();
                continue;
            }

            const auto eq = line.find('=');
            if (eq == std::string::npos)
                continue;
            const std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            while (!val.empty() && (val.front() == ' '))
                val.erase(0, 1);

            if (current_event.empty())
            {
                if (key == "version")
                    m.version = std::atoi(val.c_str());
                else if (key == "default")
                    m.default_theme = strip_quotes(val);
                else if (key == "master_volume")
                    m.limits.master_volume = std::atof(val.c_str());
                else if (key == "queue_capacity")
                    m.limits.queue_capacity = std::strtoul(val.c_str(), nullptr, 10);
                else if (key == "rate_limit_per_event_ms")
                    m.limits.rate_limit_ms = std::strtoul(val.c_str(), nullptr, 10);
                continue;
            }

            SoundEvent &ev = m.events[current_event];
            ev.id = current_event;
            if (key == "modern")
                ev.modern_path = strip_quotes(val);
            else if (key == "retro")
                ev.retro_path = strip_quotes(val);
            else if (key == "volume")
                ev.volume = std::atof(val.c_str());
            else if (key == "priority")
                ev.priority = std::atoi(val.c_str());
        }

        m.ok = m.version > 0 && !m.events.empty();
        return m;
    }

    // ---------------- headless backend ----------------
    bool HeadlessSink::play([[maybe_unused]] const SoundEvent &ev, const Snd8Sequence *retro, std::string &err)
    {
        if (!retro || !retro->ok)
        {
            err = "no valid retro sequence";
            return false;
        }
        // deterministic fold of all register ops into the hash
        for (const auto &op : retro->ops)
        {
            _hash = _hash * 1099511628211ull ^ (uint64_t(op.voice) << 24) ^ (uint64_t(op.control) << 16) ^ (uint64_t(op.freq) << 8) ^ op.vol;
            _samples += op.wait_ms * 44100u / 1000u; // samples at 44.1 kHz
        }
        return true;
    }

    // ---------------- service / router ----------------
    SystemSoundService &SystemSoundService::instance()
    {
        static SystemSoundService s;
        return s;
    }

    bool SystemSoundService::load_manifest_text(const std::string &toml_text, std::string &err)
    {
        _manifest = parse_manifest(toml_text);
        if (!_manifest.ok)
        {
            err = "manifest invalid (version/events missing)";
            return false;
        }
        _theme = _manifest.default_theme.empty() ? "retro-8bit" : _manifest.default_theme;
        return true;
    }

    static uint64_t now_ms()
    {
        return uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count());
    }

    void SystemSoundService::play(std::string_view event_id)
    {
        // boot-safety: never block, never throw — failures become log entries
        if (!_enabled)
            return;
        const auto it = _manifest.events.find(std::string(event_id));
        if (it == _manifest.events.end())
        {
            ++_dropped;
            return;
        } // silent fallback + count
        const SoundEvent &ev = it->second;

        // rate limit per event bucket (bounded buckets; IDs hash into 8 slots)
        const size_t slot = std::hash<std::string_view>{}(event_id) & 7;
        const uint64_t now = now_ms();
        if (now - _last_play_ms[slot] < _manifest.limits.rate_limit_ms)
        {
            ++_dropped;
            return;
        }
        _last_play_ms[slot] = now;

        // theme selection
        if (_theme == "silent")
            return;
        if (_theme == "modern" && ev.modern_path.empty())
            return;

        // load retro asset when needed
        Snd8Sequence seq;
        if (_theme != "modern" && !ev.retro_path.empty())
        {
            std::ifstream f(ev.retro_path, std::ios::binary);
            if (f)
            {
                std::ostringstream ss;
                ss << f.rdbuf();
                seq = parse_snd8(ss.str(), std::string(event_id));
            }
        }

        std::string err;
        if (_backend && !_backend->play(ev, seq.ok ? &seq : nullptr, err))
            ++_dropped;
    }

} // namespace osbex::audio
