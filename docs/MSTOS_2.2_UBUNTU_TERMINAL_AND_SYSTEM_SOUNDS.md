# MSTOS 2.2 — Ubuntu/GNOME-Style Terminal and System Sound Specification

**Project:** CPM-8BIT / MSTOS 2.2  
**Subsystem:** Linux desktop, terminal, system sound, and UI audio  
**Reference:** Ubuntu/GNOME-style desktop conventions + CPM-8BIT Atari-2600-inspired 8-bit audio engine  
**Status:** Architecture and implementation specification  
**Code policy:** All implementation code, sound assets, interfaces, and test data created for CPM-8BIT must be independently authored or properly licensed.

## 1. Purpose

MSTOS 2.2 will provide a Linux-based graphical desktop with a familiar modern workflow. Ubuntu/GNOME is a reference for desktop conventions, but MSTOS is not required to ship GNOME itself.

The system should have a dedicated sound event for important operating-system states, including boot, login, desktop startup, terminal events, application launch, warnings, errors, shutdown, recovery, diagnostics, and other user-visible events.

The same event system must work with the CPM-8BIT 8-bit audio device so the machine can produce short retro-style sounds while also supporting a modern Linux audio backend.

## 2. Desktop Architecture

```text
Windows Host
    ↓
CPM-8BIT Virtual Machine
    ↓
Linux Kernel
    ↓
MSTOS 2.2 Services
    ↓
MSTOS Desktop Shell
    ├── Terminal
    ├── File Manager
    ├── Settings
    ├── System Monitor
    ├── Hardware Monitor
    ├── Storage Manager
    ├── Network Manager
    └── Application Launcher
             ↓
       System Sound Service
             ↓
       Audio Event Router
          ├── CPM8-AUDIO8
          └── Linux/Host Audio Backend
```

## 3. Ubuntu/GNOME-Style Desktop Features

The MSTOS desktop should provide the following familiar concepts without copying proprietary implementation code:

- Application launcher.
- Top/system status area.
- Notifications.
- Window management.
- Workspace support.
- Settings application.
- Terminal application.
- File manager.
- Network controls.
- Audio controls.
- Power controls.
- User/session controls.
- Accessibility controls.
- Search.
- System information.

The visual design, implementation, APIs, and assets should be original CPM-8BIT/MSTOS work.

## 4. Terminal

The terminal is a first-class MSTOS application.

Required capabilities:

- Shell session.
- Multiple terminal windows/tabs.
- Copy/paste.
- Scrollback.
- Keyboard shortcuts.
- UTF-8 text.
- Configurable font size.
- Configurable colors/theme.
- Command history.
- Environment variables.
- Process control.
- File-system navigation.
- Secure host/guest boundary.
- Optional CP/M terminal session.

### Terminal audio events

Terminal sounds should normally be subtle and optional.

| Event | Sound ID | Default |
|---|---|---|
| Command accepted | `terminal.command` | Off |
| Command completed | `terminal.complete` | Off |
| Command failed | `terminal.error` | On, low volume |
| Bell | `terminal.bell` | On |
| Tab opened | `terminal.tab_open` | Off |
| Tab closed | `terminal.tab_close` | Off |
| Search match | `terminal.search` | Off |
| Permission denied | `terminal.permission_denied` | On, low volume |
| CP/M command | `terminal.cpm_command` | Optional |

## 5. System Sound Event Model

Every system sound is identified by a stable string ID.

```text
sound.boot.power_on
sound.boot.post_pass
sound.boot.post_fail
sound.boot.loader_start
sound.boot.loader_success
sound.boot.loader_fail
sound.kernel.start
sound.kernel.ready
sound.desktop.start
sound.desktop.ready
sound.desktop.notification
sound.desktop.login
sound.desktop.logout
sound.desktop.workspace_change
sound.desktop.window_open
sound.desktop.window_close
sound.desktop.menu_open
sound.desktop.menu_close
sound.app.launch
sound.app.close
sound.app.error
sound.app.warning
sound.system.info
sound.system.error
sound.system.permission_denied
sound.system.device_connected
sound.system.device_disconnected
sound.network.connected
sound.network.disconnected
sound.storage.mounted
sound.storage.unmounted
sound.storage.error
sound.download.complete
sound.update.available
sound.update.complete
sound.security.warning
sound.recovery.start
sound.recovery.success
sound.recovery.failure
sound.shutdown.start
sound.shutdown.complete
sound.reboot.start
sound.debug.breakpoint
sound.debug.exception
sound.cpm.disk_insert
sound.cpm.disk_remove
sound.cpm.program_start
sound.cpm.program_exit
sound.audio.muted
sound.audio.unmuted
```

## 6. Dedicated Sound File Per Event

Every important event has its own sound asset entry.

The repository should eventually use a structure such as:

```text
assets/
└── audio/
    ├── boot/
    │   ├── power-on.wav
    │   ├── post-pass.wav
    │   ├── post-fail.wav
    │   ├── loader-start.wav
    │   ├── loader-success.wav
    │   └── loader-fail.wav
    ├── kernel/
    │   ├── start.wav
    │   └── ready.wav
    ├── desktop/
    │   ├── start.wav
    │   ├── ready.wav
    │   ├── login.wav
    │   ├── logout.wav
    │   ├── notification.wav
    │   ├── workspace-change.wav
    │   ├── window-open.wav
    │   ├── window-close.wav
    │   ├── menu-open.wav
    │   └── menu-close.wav
    ├── terminal/
    │   ├── bell.wav
    │   ├── error.wav
    │   ├── permission-denied.wav
    │   ├── tab-open.wav
    │   └── tab-close.wav
    ├── applications/
    │   ├── launch.wav
    │   ├── close.wav
    │   ├── warning.wav
    │   └── error.wav
    ├── network/
    │   ├── connected.wav
    │   └── disconnected.wav
    ├── storage/
    │   ├── mounted.wav
    │   ├── unmounted.wav
    │   └── error.wav
    ├── security/
    │   └── warning.wav
    ├── recovery/
    │   ├── start.wav
    │   ├── success.wav
    │   └── failure.wav
    ├── shutdown/
    │   ├── start.wav
    │   └── complete.wav
    ├── reboot/
    │   └── start.wav
    ├── debug/
    │   ├── breakpoint.wav
    │   └── exception.wav
    └── cpm/
        ├── disk-insert.wav
        ├── disk-remove.wav
        ├── program-start.wav
        └── program-exit.wav
```

The filenames are examples. The final asset manifest is authoritative.

## 7. Two Audio Asset Modes

Each sound event can have two representations:

### Modern asset

A normal short audio asset such as PCM/WAV or another supported format.

### 8-bit asset

A compact register/event description for the CPM8-AUDIO8 device.

Example:

```toml
[id]
name = "sound.boot.post_pass"

[modern]
file = "audio/boot/post-pass.wav"
volume = 0.45

[retro]
control0 = 0x03
frequency0 = 0x0C
volume0 = 0x0F
control1 = 0x00
frequency1 = 0x00
volume1 = 0x00
duration_ms = 90
```

This allows the same event to have a modern presentation or an 8-bit presentation.

## 8. Audio Themes

MSTOS should support selectable sound themes:

```text
modern
retro-8bit
osborne-inspired
silent
custom
```

### Modern

Uses the Linux/host audio backend and normal sound assets.

### Retro 8-bit

Uses the CPM8-AUDIO8 device and compact synthesized event definitions.

### Osborne-inspired

Uses short, restrained tones designed around the computer's historical identity.

### Silent

Disables nonessential system sounds while preserving diagnostic logging.

### Custom

Allows users to provide licensed compatible sound assets.

## 9. Boot Sound Sequence

The boot process should have independently addressable events:

```text
Power On
  ↓ sound.boot.power_on
Firmware/POST
  ↓ sound.boot.post_pass
Boot Loader
  ↓ sound.boot.loader_start
Linux Kernel
  ↓ sound.kernel.start
MSTOS Services
  ↓ sound.kernel.ready
Desktop
  ↓ sound.desktop.start
Desktop Ready
  ↓ sound.desktop.ready
```

Failure paths use separate sounds:

```text
POST failure       → sound.boot.post_fail
Boot failure       → sound.boot.loader_fail
Kernel failure     → system error/recovery path
Recovery failure   → sound.recovery.failure
```

Boot audio must never block the boot process.

## 10. Shutdown and Restart Sounds

Shutdown:

```text
User selects Shutdown
  ↓ sound.shutdown.start
Services stop
  ↓
Filesystem sync
  ↓
Audio fades/stops
  ↓ sound.shutdown.complete
Power off
```

Restart:

```text
User selects Restart
  ↓ sound.reboot.start
Services stop
  ↓
Firmware restart
```

If the system crashes, the normal shutdown sound must not be falsely emitted.

## 11. Notification Sounds

Notifications have priority classes:

| Priority | Example | Audio |
|---|---|---|
| 0 | Informational | Optional |
| 1 | Normal notification | Low |
| 2 | Warning | Moderate |
| 3 | Error | Higher |
| 4 | Critical/recovery | High but capped |

Critical sounds must still respect a master volume limit.

## 12. Sound Event Router

The system sound service receives events rather than letting applications access the audio device directly.

```cpp
namespace MSTOS::Audio {

class SystemSoundService {
public:
    virtual void play(std::string_view event_id) = 0;
    virtual void stop(std::string_view event_id) = 0;
    virtual void set_theme(std::string_view theme) = 0;
    virtual void set_enabled(bool enabled) = 0;
};

}
```

Applications request:

```cpp
sound.play("sound.app.launch");
```

The router determines:

1. Whether sound is enabled.
2. Which theme is active.
3. Which asset is assigned.
4. Whether the event is permitted.
5. Priority and volume.
6. Whether an existing sound should be stopped or mixed.
7. Which audio backend should receive the event.

## 13. 8-Bit Event Synthesis

For the retro theme, a sound event becomes a short register sequence.

Example boot-success chirp:

```text
Voice 0
CONTROL   = 0x03
FREQUENCY = 0x08
VOLUME    = 0x0C

wait 60 ms

FREQUENCY = 0x0C

wait 60 ms

VOLUME    = 0x00
```

Example error tone:

```text
Voice 0
CONTROL   = 0x05
FREQUENCY = 0x18
VOLUME    = 0x0E

wait 100 ms

FREQUENCY = 0x10

wait 100 ms

VOLUME    = 0x00
```

These are CPM-8BIT sound definitions, not copied source code.

## 14. Sound Manifest

A versioned manifest controls all event-to-asset mappings.

Suggested location:

```text
config/audio/system-sounds.toml
```

Example:

```toml
version = 1

[theme]
default = "retro-8bit"

[sounds."sound.boot.power_on"]
modern = "audio/boot/power-on.wav"
retro = "audio/boot/power-on.8snd"
volume = 0.40
priority = 3

[sounds."sound.boot.post_pass"]
modern = "audio/boot/post-pass.wav"
retro = "audio/boot/post-pass.8snd"
volume = 0.35
priority = 2

[sounds."sound.desktop.ready"]
modern = "audio/desktop/ready.wav"
retro = "audio/desktop/ready.8snd"
volume = 0.30
priority = 2
```

## 15. Proposed `.8snd` Format

CPM-8BIT can define an original text/binary sound-sequence format called `.8snd`.

Conceptual text form:

```text
8SND 1
VOICE 0 CONTROL 03 FREQ 0C VOL 0F
WAIT 060
VOICE 0 FREQ 10
WAIT 060
VOICE 0 VOL 00
END
```

Required fields:

- Format version.
- Event identifier.
- Device profile.
- Voice operations.
- Timing.
- Optional envelope.
- Optional loop count.
- Optional metadata.
- Integrity hash in packaged releases.

The binary form may later be introduced for embedded hardware.

## 16. Sound Asset Validation

Build tools must validate:

- Event ID exists.
- Referenced files exist.
- Asset format is supported.
- Duration is within policy.
- Volume is within range.
- Retro register values are valid.
- `.8snd` syntax is valid.
- No required boot event is missing.
- No two mandatory events incorrectly share an asset unless explicitly permitted.

## 17. Boot-Critical Audio Rules

Audio must never make the machine unbootable.

If an audio asset fails:

```text
missing sound
    ↓
log warning
    ↓
use silent fallback
    ↓
continue boot
```

A corrupted audio device must not prevent access to recovery mode.

## 18. Terminal Sound Controls

Settings should include:

```text
System Sounds       [On/Off]
Terminal Bell       [On/Off]
Boot Sounds         [On/Off]
Notification Sounds [On/Off]
Error Sounds        [On/Off]
Retro Audio         [On/Off]
Master Volume       [0–100]
Sound Theme         [Modern/Retro/Osborne/Silent/Custom]
```

## 19. CP/M Compatibility Sounds

The CP/M environment may expose optional events:

```text
CP/M disk inserted
CP/M disk removed
CP/M application started
CP/M application exited
CP/M error
CP/M console bell
```

These are implemented through the CPM-8BIT compatibility layer rather than modifying historical CP/M software.

## 20. Atari 2600 Relationship

The Atari 2600-inspired audio engine is the **retro synthesis backend**, not the complete desktop audio architecture.

```text
MSTOS System Sound Event
          ↓
    Event Router
       /     \
      /       \
 Modern      Retro
 Backend     CPM8-AUDIO8
              ↓
       8-bit synthesis
```

This makes it possible to retain the Atari-style 8-bit sound character while supporting normal Linux desktop audio.

## 21. Linux Audio Integration

The Linux side should use an abstraction layer rather than tying MSTOS directly to a single desktop audio implementation.

Recommended architecture:

```text
MSTOS::Audio
    ↓
MSTOS Audio Service
    ↓
Linux Audio Adapter
    ↓
Configured Linux audio subsystem
    ↓
Host/physical audio device
```

The adapter should support a headless backend for testing.

## 22. Sound Priority and Mixing

System events must use bounded queues and priorities.

Rules:

- Critical errors may interrupt informational sounds.
- Repeated notifications should be rate limited.
- Terminal bells should not flood the audio queue.
- Boot sounds may be marked exclusive.
- Shutdown sounds should stop before power-off.
- Audio queues must have fixed or bounded capacity.

## 23. Accessibility

Every sound notification must have a non-audio equivalent where the event carries important information.

Examples:

```text
Audio error → visual notification + log entry
Device connected → notification + system log
Boot failure → diagnostic screen + log
Security warning → visible warning + log
```

Users must be able to disable nonessential audio without losing system information.

## 24. Testing

Required automated tests:

- Sound manifest parsing.
- Missing asset detection.
- Invalid event detection.
- Theme switching.
- Event routing.
- Priority handling.
- Volume limits.
- Queue overflow handling.
- Boot audio fallback.
- Shutdown audio behavior.
- Retro `.8snd` parsing.
- 8-bit register generation.
- Deterministic playback.
- Snapshot/restore.
- Headless playback.
- Linux audio adapter tests.
- Windows host audio adapter tests.

## 25. Recommended Repository Layout

```text
config/
└── audio/
    └── system-sounds.toml

assets/
└── audio/
    ├── boot/
    ├── kernel/
    ├── desktop/
    ├── terminal/
    ├── applications/
    ├── network/
    ├── storage/
    ├── security/
    ├── recovery/
    ├── shutdown/
    ├── reboot/
    ├── debug/
    └── cpm/

docs/
├── AUDIO_SPEC.md
└── MSTOS_2.2_UBUNTU_TERMINAL_AND_SYSTEM_SOUNDS.md

src/
└── audio/
    ├── system_sound_service.cpp
    ├── sound_event_router.cpp
    ├── sound_manifest.cpp
    ├── sound_theme.cpp
    ├── snd8_parser.cpp
    ├── retro_sound_backend.cpp
    └── linux_audio_backend.cpp

tests/
└── audio/
    ├── sound_manifest_tests.cpp
    ├── sound_router_tests.cpp
    ├── snd8_tests.cpp
    └── retro_backend_tests.cpp
```

## 26. Acceptance Criteria

The system is ready when:

1. MSTOS boots without requiring audio hardware.
2. The desktop provides an Ubuntu/GNOME-style modern workflow.
3. The terminal is fully usable without sound.
4. Important system events have independent sound IDs.
5. Required event assets are validated during the build.
6. Retro 8-bit events can use the CPM8-AUDIO8 device.
7. Modern events can use the Linux/host audio backend.
8. Users can select Modern, Retro, Osborne-inspired, Silent, or Custom themes.
9. Boot and recovery continue safely if an audio asset fails.
10. Headless CI can test the complete sound-event system.
11. Audio state can participate in deterministic VM snapshots.
12. No proprietary or third-party source code is copied into the project.

## 27. Final System Concept

The resulting CPM-8BIT/MSTOS 2.2 experience is:

```text
                 MSTOS 2.2
                     │
          ┌──────────┴──────────┐
          │                     │
     Linux Desktop          System Events
          │                     │
   Ubuntu/GNOME-style       Sound Router
    workflow concepts           │
          │             ┌───────┴────────┐
      Terminal          │                │
      Files             │                │
      Settings       Modern          Retro 8-bit
      Launcher       Audio           Audio
      Windows           │                │
                     Linux/Host      CPM8-AUDIO8
                                      │
                              Atari-2600-inspired
                                programmable sound
```

The result preserves the original 8-bit computer identity while providing a modern Linux desktop and a dedicated sound for each important system option, boot state, loader state, terminal event, application event, notification, error, recovery action, and shutdown/restart state.