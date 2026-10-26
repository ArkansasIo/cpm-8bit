# CPM-8BIT — Atari 2600-Style 8-Bit Audio Specification

**Project:** CPM-8BIT / osbex 2.2  
**Subsystem:** 8-bit audio  
**Profile:** Atari 2600-inspired TIA-style sound  
**Status:** Architecture and implementation specification  
**Code policy:** The implementation must be independently authored. This document specifies compatible behavior and design goals; it does not reproduce third-party source code.

## 1. Purpose

CPM-8BIT will include an optional 8-bit audio subsystem inspired by the sound architecture of the Atari 2600. The goal is to reproduce the characteristic programmable-console sound model while integrating it with the CPM-8BIT virtual machine, osbex 2.2, the Windows application, the CP/M compatibility environment, and future physical hardware.

The audio device is a **two-voice programmable sound generator** with compact register-style control. It is intentionally small, deterministic, and suitable for both an emulator and a future FPGA/embedded implementation.

## 2. Design Goals

- Two programmable audio voices.
- Atari 2600/TIA-inspired register model.
- Four-bit volume control per voice.
- Five-bit frequency/divider control per voice.
- Four-bit waveform/control selection per voice.
- Deterministic audio generation.
- Configurable output sample rate.
- Windows host audio backend.
- Silent/headless mode for CI.
- Audio state included in VM snapshots.
- Audio timing tied to the guest clock.
- Independently authored implementation and tests.

## 3. Historical Reference Model

The Atari 2600 generated sound through the Television Interface Adaptor (TIA). Its audio architecture is different from modern PCM: software programs compact control registers that select tone/noise behavior, frequency division, and volume.

CPM-8BIT uses this concept as a compatibility-inspired design reference. The CPM-8BIT device, register map, API, implementation, and tests are original project work.

## 4. Virtual Audio Device

Device name:

```text
CPM8-AUDIO8
```

Recommended identifier:

```text
cpm8.audio.tia8
```

Two voices are exposed:

```text
Voice 0
Voice 1
```

Each voice provides:

```text
CONTROL    4 bits
FREQUENCY  5 bits
VOLUME     4 bits
```

## 5. Register Model

The logical register model is:

| Register | Width | Description |
|---|---:|---|
| `AUDC0` | 8-bit | Voice 0 waveform/control; low 4 bits active |
| `AUDF0` | 8-bit | Voice 0 frequency divider; low 5 bits active |
| `AUDV0` | 8-bit | Voice 0 volume; low 4 bits active |
| `AUDC1` | 8-bit | Voice 1 waveform/control; low 4 bits active |
| `AUDF1` | 8-bit | Voice 1 frequency divider; low 5 bits active |
| `AUDV1` | 8-bit | Voice 1 volume; low 4 bits active |
| `AUCTRL` | 8-bit | Global enable/mute/status controls |
| `AUDIO_STATUS` | 8-bit | Device status and error flags |

Final MMIO addresses are assigned in `IO_MAP.md` and versioned.

## 6. Control Values

CPM-8BIT defines a stable four-bit control mapping:

| Control | CPM-8BIT mode |
|---:|---|
| `0x0` | Silence |
| `0x1` | Low-frequency tone |
| `0x2` | Square tone |
| `0x3` | Divided square tone |
| `0x4` | Metallic/noise tone |
| `0x5` | Periodic noise |
| `0x6` | Pitched noise |
| `0x7` | Wide noise |
| `0x8` | Tone variant A |
| `0x9` | Tone variant B |
| `0xA` | Tone/noise variant A |
| `0xB` | Tone/noise variant B |
| `0xC` | Short periodic texture |
| `0xD` | Long periodic texture |
| `0xE` | Long noise texture |
| `0xF` | Noise texture variant |

These are CPM-8BIT mode names. Exact historical TIA bitstream compatibility, if later required, will be a separately versioned compatibility target.

## 7. Frequency Generation

Each voice uses a five-bit frequency/divider value:

```text
frequency_divider = AUDF & 0x1F
```

The effective oscillator period is derived from the guest audio/device clock and must be deterministic for a given machine profile and register state.

Recommended architecture:

```text
period = base_audio_clock / divisor(frequency_divider)
```

The `divisor()` function is part of the versioned device definition. Deterministic mode should use integer or fixed-point phase accumulation rather than floating-point oscillator state.

## 8. Volume

Each voice uses four-bit volume:

```text
volume = AUDV & 0x0F
```

Range:

```text
0  = silent
15 = maximum logical amplitude
```

Host-specific output scaling occurs only after the deterministic guest audio stream is generated.

## 9. Mixer

The default logical output is mono:

```text
voice0 = generate(control0, frequency0, volume0)
voice1 = generate(control1, frequency1, volume1)
output = mix(voice0, voice1)
```

The mixer must have deterministic clipping/saturation behavior. Optional stereo output may expose the two voices separately, but mono is the baseline.

## 10. Host Audio Backend

The Windows application should provide:

- Device selection.
- Sample-rate selection.
- Buffer-size selection.
- Master volume.
- Mute.
- Audio enable/disable.
- Headless mode.
- Debug waveform capture.

The guest device must not depend directly on Windows audio APIs:

```text
osbex / Emulator
      ↓
CPM8-AUDIO8
      ↓
Deterministic sample queue
      ↓
Host audio adapter
      ↓
Windows audio output
```

## 11. Headless and CI Mode

CI must exercise the audio device without requiring a physical sound card.

```text
Guest audio → deterministic sample buffer → test/hash sink
```

Tests should cover register state, sample count, oscillator transitions, mixer output, silence, and snapshot/restore.

## 12. Snapshot State

Snapshots include:

- `AUDC0`, `AUDF0`, `AUDV0`.
- `AUDC1`, `AUDF1`, `AUDV1`.
- Global audio state.
- Oscillator phase/state.
- Noise-generator state.
- Mixer state where required.
- Device clock position.
- Audio format/version identifier.

Snapshot restore must reproduce the same subsequent guest audio sequence in deterministic mode.

## 13. osbex C++23 Audio API

The original public API uses:

```cpp
namespace osbex::Audio {
    class Device;
    class Voice;
    class Mixer;
    class SampleStream;
    class AudioConfig;
}
```

Conceptual usage:

```cpp
auto audio = osbex.audio().device("cpm8.audio.tia8");
audio.voice(0).set_control(0x03);
audio.voice(0).set_frequency(12);
audio.voice(0).set_volume(15);
```

The final ABI defines ownership, threading, errors, permissions, and versioning.

## 14. CP/M Integration

CP/M programs may access sound through an optional CPM-8BIT sound utility, driver, console command, demonstration program, or machine-profile service. These interfaces are original unless a documented historical interface is deliberately reimplemented for compatibility.

## 15. Atari 2600-Inspired Development Profile

```toml
[audio]
enabled = true
model = "tia8-inspired"
voices = 2
sample_rate = 44100
channels = 1
deterministic = true
```

This profile is an audio-development target and is not a claim of complete Atari 2600 emulation.

## 16. Future Atari 2600 Compatibility Profile

A future full Atari 2600 profile should remain isolated from the core osbex architecture and may contain:

```text
profiles/
└── atari2600/
    ├── cpu/
    ├── tia/
    │   ├── video/
    │   └── audio/
    ├── riot/
    ├── cartridge/
    ├── controllers/
    └── tests/
```

Such a profile would need the required CPU, memory, TIA video/audio behavior, RIOT, cartridge mapping, controller input, and timing model.

## 17. Sound Development Tools

Planned tools:

```text
tools/audio/
├── sound-editor
├── register-monitor
├── waveform-viewer
├── audio-recorder
├── audio-player
├── tia8-converter
└── audio-test-generator
```

The editor should support control value, frequency divider, volume, duration, envelope, voice sequencing, and noise/periodic modes.

## 18. Example 8-Bit Sound Sequence

```text
Voice 0:
  CONTROL   = 0x03
  FREQUENCY = 0x0C
  VOLUME    = 0x0F

Voice 1:
  CONTROL   = 0x05
  FREQUENCY = 0x18
  VOLUME    = 0x08

wait 120 ms

Voice 0:
  FREQUENCY = 0x10
  VOLUME    = 0x0C

wait 120 ms

Voice 0:
  VOLUME    = 0x00
Voice 1:
  VOLUME    = 0x00
```

This is a CPM-8BIT test sequence, not copied historical source code.

## 19. Testing

Required tests:

- Register masking.
- Reset values.
- Volume range.
- Frequency range.
- Control-mode selection.
- Silence mode.
- Voice independence.
- Two-voice mixing.
- Saturation behavior.
- Deterministic output.
- Snapshot/restore.
- Pause/resume.
- Host audio disconnect/reconnect.
- Headless operation.
- Invalid register access.
- Device-version compatibility.

## 20. Performance Targets

Initial targets:

- Two voices with negligible VM CPU overhead.
- No audio glitches under normal host load.
- Deterministic mode suitable for automated regression tests.
- Stable configurable host latency.
- No unbounded audio-queue memory growth.

## 21. Physical Hardware Migration

The software model becomes the reference for a future FPGA or embedded implementation:

```text
C++23 software model
        ↓
Timing-tested emulator
        ↓
FPGA RTL implementation
        ↓
SoC/embedded audio device
        ↓
Physical CPM-8BIT computer
```

## 22. CPM-8BIT Integration

```text
Windows
  ↓
CPM-8BIT Emulator
  ↓
Virtual Hardware
  ├── CPU
  ├── RAM
  ├── Storage
  ├── Display
  ├── Keyboard
  ├── Network
  └── CPM8-AUDIO8
          ↓
      2 programmable voices
          ↓
      osbex::Audio
          ↓
      Windows Audio Backend
```

This preserves the project's 8-bit identity while allowing osbex 2.2 to provide a modern 64-bit environment.

## 23. Acceptance Criteria

The audio subsystem is ready when:

1. The device initializes deterministically.
2. Both voices can be independently programmed.
3. Control, frequency, and volume registers behave as specified.
4. Mixing is stable and clipping is controlled.
5. Windows playback works through a host adapter.
6. Headless CI works without an audio device.
7. Snapshot/restore reproduces deterministic audio state.
8. The device is exposed through the original osbex C++23 API.
9. Automated tests cover register and timing behavior.
10. No implementation source is copied from Atari, emulators, operating systems, or other third-party projects.

## 24. Related Documentation

- `docs/CPM-8BIT_osbex-2.2_ALL-IN-ONE-SPEC.md`
- `docs/IO_MAP.md`
- `docs/MEMORY_MAP.md`
- `docs/VIRTUAL_MACHINE_SPEC.md`
- `docs/AUDIO_SPEC.md`
- `docs/CLOCK_TIMER_SPEC.md`
- `docs/SNAPSHOT_SPEC.md`
- `docs/TESTING.md`
- `docs/PERFORMANCE.md`

---

**Implementation principle:** use documented machine behavior as a compatibility reference, while independently authoring the CPM-8BIT audio engine, APIs, tests, tools, and hardware description.
