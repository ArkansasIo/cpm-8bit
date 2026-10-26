# docs/AUDIO_SPEC.md — CPM-8BIT 8-bit Audio (TIA-inspired, two voices)

Device: **CPM8-AUDIO8** — id `cpm8.audio.tia8`. Independently authored;
behavior is a compatibility *reference*, no third-party code.

## Register model (final MMIO addresses versioned in IO_MAP.md)

| Register      | Width | Meaning                                   |
|---------------|-------|-------------------------------------------|
| AUDC0/1       | 8-bit | voice 0/1 control, low 4 bits active      |
| AUDF0/1       | 8-bit | voice 0/1 frequency divider, low 5 bits   |
| AUDV0/1       | 8-bit | voice 0/1 volume, low 4 bits              |
| AUCTRL        | 8-bit | global enable/mute/status                 |
| AUDIO_STATUS  | 8-bit | status + error flags                      |

## Control modes (0x0–0xF)

0x0 silence, 0x1 low-freq tone, 0x2 square, 0x3 divided square, 0x4 metallic,
0x5 periodic noise, 0x6 pitched noise, 0x7 wide noise, 0x8/0x9 tone variants,
0xA/0xB tone-noise variants, 0xC/0xD periodic textures, 0xE/0xF noise textures.

## Generation

- `freq_div = AUDF & 0x1F`; `period = base_audio_clock / divisor(freq_div)`
- deterministic: fixed-point phase accumulation, no float oscillator state
- volume `AUDV & 0x0F` (0 silent … 15 max); host scaling after guest stream
- mixer: mono `mix(v0, v1)` with deterministic saturation; stereo optional

## Stack

```
osbex/Emulator → CPM8-AUDIO8 → deterministic sample queue
              → host audio adapter → Windows output
```

Headless/CI: guest audio → sample buffer → test/hash sink (no sound card).

## Snapshot state

AUDC/F/V ×2, global state, oscillator phase, noise-generator state, mixer,
device clock position, format/version id. Restore must reproduce the same
subsequent sample sequence in deterministic mode; incompatible versions are
rejected safely.

## osbex C++23 API

```cpp
namespace osbex::Audio { class Device; class Voice; class Mixer;
                         class SampleStream; class AudioConfig; }

auto audio = osbex.audio().device("cpm8.audio.tia8");
audio.voice(0).set_control(0x03);
audio.voice(0).set_frequency(12);
audio.voice(0).set_volume(15);
```

## Profile

```ini
[audio]
enabled = true
model = "tia8-inspired"
voices = 2
sample_rate = 44100
channels = 1
deterministic = true
```

## Tools (planned)

`tools/audio/`: sound-editor, register-monitor, waveform-viewer,
audio-recorder/player, tia8-converter, audio-test-generator.

## Tests

register masking, reset values, volume/frequency range, control-mode
selection, silence, voice independence, two-voice mixing, saturation,
deterministic output, snapshot/restore, pause/resume, host
disconnect/reconnect, headless, invalid register access, version compat.

## Acceptance

deterministic init; both voices independently programmable; registers behave
as specified; stable mixing/clipping; Windows playback via host adapter;
headless CI works; snapshot/restore deterministic; exposed via osbex C++23
API; automated register/timing tests; no copied implementation source.
