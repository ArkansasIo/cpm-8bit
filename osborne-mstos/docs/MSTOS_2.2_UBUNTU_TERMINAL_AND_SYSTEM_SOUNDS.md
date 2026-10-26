# osbex 2.2 — Ubuntu/GNOME-Style Terminal and System Sounds

Reference for desktop conventions: Ubuntu/GNOME *concepts only* — all
implementation, assets and APIs are original CPM-8BIT/osbex work.

## Architecture

```
osbex Desktop Shell (terminal, files, settings, monitors, launcher)
        ↓
System Sound Service → Audio Event Router
        ├── Modern backend  (Linux/host audio)
        └── Retro backend   (CPM8-AUDIO8, .8snd synthesis)
```

## Event IDs

Stable string IDs, e.g. `sound.boot.power_on`, `sound.boot.post_fail`,
`sound.desktop.ready`, `sound.terminal.bell`, `sound.app.launch`,
`sound.network.connected`, `sound.storage.error`, `sound.recovery.start`,
`sound.shutdown.complete`, `sound.cpm.disk_insert`, … (full list in
`config/audio/system-sounds.toml`, which is authoritative).

## Two asset modes per event

- **modern** — PCM/WAV asset for the Linux/host backend.
- **retro** — `.8snd` register sequence for CPM8-AUDIO8.

```
[retro]
control0 = 0x03  frequency0 = 0x0C  volume0 = 0x0F
duration_ms = 90
```

## Themes

`modern` | `retro-8bit` | `osborne-inspired` | `silent` | `custom`

## Priorities

0 informational … 4 critical/recovery. Critical may interrupt informational
sounds; notifications are rate-limited; queues are bounded; boot audio never
blocks boot; a missing asset falls back to silence + log entry.

## .8snd format (original)

```
8SND 1
VOICE 0 CONTROL 03 FREQ 0C VOL 0F
WAIT 060
VOICE 0 FREQ 10
WAIT 060
VOICE 0 VOL 00
END
```

## Layout

- `config/audio/system-sounds.toml` — versioned manifest
- `assets/audio/**` — event assets (`.wav` modern, `.8snd` retro)
- `osbex/audio/` — service, router, manifest, `.8snd` parser, retro backend
- `tests/audio/` — manifest/router/parser/backend tests

## Acceptance

Boots without audio hardware; terminal usable silently; independent event
IDs; build-time asset validation; retro + modern backends; theme selection;
safe fallback on asset failure; headless CI coverage; deterministic snapshot
state; no third-party code copied.
