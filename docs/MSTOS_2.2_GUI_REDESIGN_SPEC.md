# MSTOS 2.2 — Redesigned Ubuntu/GNOME-Style GUI Specification

## 1. Purpose

This document defines the visual and interaction design for the new **MSTOS 2.2 Desktop GUI**. The design uses the usability principles of a free Linux desktop such as Ubuntu's GNOME-based environment, but it is **not a copy of Ubuntu or GNOME**. MSTOS has its own shell, identity, navigation model, window chrome, icons, sounds, settings, and system applications.

The goal is a familiar modern Linux desktop while preserving the Osborne/CPM-8BIT identity.

## 2. Design Principles

- Familiar Linux desktop behavior.
- Original MSTOS visual language.
- Keyboard-first and mouse-friendly operation.
- Fast startup and low resource consumption.
- Clear separation between system, applications, developer tools, and legacy CP/M software.
- Responsive layout for the Windows virtual machine and future physical hardware.
- Accessible typography, contrast, scaling, and keyboard navigation.
- Every major system state has a corresponding visual and optional audio state.

## 3. Desktop Identity

### Product

**MSTOS 2.2 Desktop**

### Visual character

- Dark command-center base theme.
- Industrial blue-gray surfaces.
- High-contrast text.
- Thin technical borders.
- Subtle CRT/retro-computing references.
- Modern rounded panels used sparingly.
- Monospace typography for terminal/system information.
- Standard proportional font for applications.
- Optional Osborne-inspired display mode.

The interface should feel like a modern operating system built for a computer descended from the Osborne concept, rather than a generic Linux distribution.

## 4. Main Screen Layout

```text
+-----------------------------------------------------------------------+
| MSTOS | Activities | Applications | Workspace 1 | Status | 12:30 AM |
+-----------------------------------------------------------------------+
|                                                                       |
|                                                                       |
|                         DESKTOP / WORKSPACE                           |
|                                                                       |
|      +----------------------+     +------------------------------+    |
|      | System Monitor       |     | Terminal                     |    |
|      | CPU  18%             |     | mst@mst:~$                   |    |
|      | RAM  42%             |     |                              |    |
|      | Disk 12%             |     |                              |    |
|      +----------------------+     +------------------------------+    |
|                                                                       |
|                                                                       |
+-----------------------------------------------------------------------+
| Launcher | Files | Terminal | Settings | CP/M | Dev | Help | Apps      |
+-----------------------------------------------------------------------+
```

The exact arrangement is configurable. The shell must support both a traditional desktop and a clean workspace with no permanent desktop icons.

## 5. Top System Bar

The top bar provides persistent system controls.

### Left side

- MSTOS logo.
- Activities/workspace control.
- Applications menu.
- Current application title when appropriate.

### Center

- Optional active-window title.
- Workspace indicator.
- Search entry when the Activities view is open.

### Right side

- Network status.
- Audio status.
- Battery/power status when physical hardware provides it.
- Storage activity indicator.
- VM/guest status.
- Notifications.
- Clock/calendar.
- User/session menu.

## 6. Application Launcher

The launcher is MSTOS's equivalent of a modern Linux application overview.

### Sections

1. Favorites
2. Recent applications
3. All applications
4. System tools
5. Development
6. CP/M / Osborne
7. Games
8. Utilities
9. Administration

### Launcher behavior

- Search-as-you-type.
- Keyboard navigation.
- Application categories.
- Pin/unpin favorites.
- Recently used applications.
- Right-click/context actions.
- Launch applications in the current or new workspace.

## 7. Dock / Task Launcher

MSTOS may provide an optional dock at the bottom or side of the screen.

Each item supports:

- Launch.
- Focus.
- Minimize.
- Restore.
- Close.
- Multiple-window indicator.
- Progress indicator.
- Notification badge.

The dock is optional so the desktop can operate in a GNOME-like workspace mode without permanently occupying screen space.

## 8. Window Manager

The MSTOS window manager/shell layer controls application windows.

### Required operations

- Move.
- Resize.
- Minimize.
- Maximize.
- Fullscreen.
- Close.
- Snap left/right/top/bottom.
- Tile two or more windows.
- Workspace movement.
- Always-on-top.
- Window switching.
- Keyboard window navigation.

### Window states

```text
NORMAL
MINIMIZED
MAXIMIZED
FULLSCREEN
TILED_LEFT
TILED_RIGHT
TILED_TOP
TILED_BOTTOM
HIDDEN
CLOSING
```

## 9. Workspaces

MSTOS uses virtual workspaces for application organization.

Example:

- Workspace 1 — General
- Workspace 2 — Development
- Workspace 3 — CP/M
- Workspace 4 — System Administration
- Workspace 5 — Games

Workspace count can be dynamic.

## 10. File Manager

### MSTOS Files

The file manager provides:

- Home directory.
- System files.
- Applications.
- Documents.
- Downloads.
- Removable storage.
- Disk images.
- CP/M disks.
- Shared Windows folders.
- Virtual-machine storage.

### Views

- Icons.
- Compact list.
- Detailed list.
- Tree.
- Terminal-integrated view.

### Context menu

- Open.
- Open With.
- New Folder.
- Rename.
- Copy.
- Cut.
- Paste.
- Delete.
- Properties.
- Open Terminal Here.
- Mount Disk Image.
- Run as Application where permitted.

## 11. Terminal

The terminal is a first-class MSTOS application.

### Default shell

```text
mst-shell
```

### Example

```text
MSTOS 2.2 Terminal
Copyright (c) ArkansasIo

mst@mst:~$ help
mst@mst:~$ system status
mst@mst:~$ vm list
mst@mst:~$ disk list
mst@mst:~$ cpm start
mst@mst:~$ audio test
```

### Terminal features

- Tabs.
- Split panes.
- Search.
- Copy/paste.
- Scrollback.
- Unicode.
- ANSI/VT-style compatibility.
- Command history.
- Configurable font and size.
- Optional CRT appearance.
- Sound events for command completion, errors, and notifications.

## 12. Settings Application

The Settings application is organized into major system categories.

### Categories

- Appearance
- Display
- Windows
- Keyboard
- Mouse
- Touch/Input
- Audio
- Network
- Storage
- Power
- Notifications
- Privacy
- Security
- Users
- Applications
- Virtual Machine
- CP/M Compatibility
- Developer Options
- Accessibility
- Language & Region
- Date & Time
- Updates
- About MSTOS

## 13. Control Center

A compact control center is opened from the system status area.

Quick controls:

- Wi-Fi/network.
- Audio volume.
- Microphone.
- Display brightness.
- Night mode.
- Do Not Disturb.
- Bluetooth where supported.
- VM pause/resume.
- Storage status.
- Power.

## 14. Notification System

Notifications use a unified MSTOS event service.

Types:

```text
INFO
SUCCESS
WARNING
ERROR
CRITICAL
PROGRESS
SYSTEM
SECURITY
NETWORK
STORAGE
UPDATE
```

Each notification may specify a sound event from the MSTOS audio manifest.

## 15. Boot and Login Experience

### Boot sequence

```text
POWER ON
   |
   v
FIRMWARE / POST
   |
   v
MSTOS BOOT MANAGER
   |
   +---- Linux Kernel
   |
   +---- Recovery
   |
   +---- CP/M Compatibility
   |
   v
MSTOS CORE SERVICES
   |
   v
MSTOS SHELL
   |
   v
DESKTOP
```

### Visual boot stages

1. Hardware initialization.
2. Memory discovery.
3. Storage discovery.
4. CPU/VM initialization.
5. Linux boot.
6. MSTOS services.
7. Display manager.
8. Desktop shell.

Each stage has an optional sound event defined by the MSTOS sound manifest.

## 16. Loading Screen

The loading screen is intentionally distinctive rather than a standard Ubuntu splash.

### Design

- MSTOS logo.
- System status line.
- Progress indicator.
- Optional diagnostic messages.
- Hardware/VM profile.
- Small retro status display.

Example:

```text
             MSTOS 2.2
       MODERN COMPUTING SYSTEM

       [##########----------]  52%

       Initializing virtual storage...
       CPU: Z80 COMPAT / MODERN-64
       RAM: 64 MB
       DISPLAY: MSTOS-FB

       Press F8 for Recovery
```

## 17. Desktop Modes

### Modern Mode

Primary desktop for Windows/Linux-hosted operation.

### Classic Mode

Osborne-inspired layout with:

- Fixed menu bar.
- Compact windows.
- Monochrome-inspired theme.
- Dense information layout.

### Terminal Mode

Full-screen terminal environment.

### CP/M Mode

Dedicated CP/M/Z80 compatibility environment.

### Recovery Mode

Minimal interface for repairing the installation and inspecting logs.

## 18. Color System

The visual tokens should be centrally defined rather than embedded throughout the UI.

```text
Background       #0B1118
Surface          #111A24
Surface Raised   #172330
Border           #2A3A49
Text             #E8EEF3
Text Muted       #9BAAB8
Accent           #4DB6FF
Success          #52D273
Warning          #F0C75E
Error            #FF5C67
Critical         #FF3045
```

These values are initial design tokens and may be adjusted during visual prototyping.

## 19. Typography

### Primary UI

A modern open-source sans-serif font.

### System/terminal

A monospace font.

### Retro mode

A bitmap/monospace display font may be supplied as an optional asset.

All fonts must support the required Unicode range for the selected localization set.

## 20. Icons

MSTOS uses an original icon set.

Required icon families:

- System.
- Files.
- Terminal.
- Network.
- Audio.
- Storage.
- CPU.
- Memory.
- Display.
- Security.
- Developer.
- CP/M.
- Osborne.
- Games.
- Power.
- Recovery.

Icons should share a consistent geometry and stroke system.

## 21. System Monitor

The system monitor provides a real-time dashboard.

Metrics:

- CPU utilization.
- Guest CPU utilization.
- Host CPU utilization.
- Memory allocation.
- Guest memory usage.
- Storage I/O.
- Network traffic.
- Audio buffer status.
- VM state.
- Linux process state.
- CP/M emulator state.

## 22. VM Control Panel

Because MSTOS is initially hosted by a Windows application, the GUI exposes the virtual machine directly.

Controls:

- Start.
- Pause.
- Resume.
- Reset.
- Shutdown.
- Hard reset.
- Snapshot.
- Restore snapshot.
- Attach disk.
- Eject disk.
- Change RAM profile.
- Change CPU profile.
- Display scaling.
- Input capture.

## 23. CP/M / Osborne Control Center

Dedicated legacy-computing interface:

```text
CP/M SYSTEM

Disk A:  [MOUNTED]
Disk B:  [EMPTY]

CPU: Z80 COMPATIBILITY
RAM: 64 KB
MODE: CP/M 2.2

[ START ] [ RESET ] [ MOUNT ] [ EJECT ]
[ TERMINAL ] [ DEBUG ] [ SNAPSHOT ]
```

This keeps historical software isolated from the modern MSTOS desktop while making it easy to launch.

## 24. Audio/UI Integration

Every important GUI action can reference an MSTOS sound event.

Examples:

```text
ui.focus
ui.navigate
ui.select
ui.confirm
ui.cancel
ui.open
ui.close
ui.error
ui.warning
ui.notification
boot.start
boot.post
boot.loader
boot.kernel
boot.desktop
system.login
system.logout
system.shutdown
system.reboot
terminal.command
terminal.error
storage.mount
storage.eject
network.connect
network.disconnect
cpm.start
cpm.exit
recovery.enter
recovery.exit
```

The Atari-2600-inspired 8-bit sound system is used as a selectable sound theme, not as a requirement for every modern installation.

## 25. Accessibility

Required options:

- UI scaling from 100% through high-DPI sizes.
- High-contrast theme.
- Reduced motion.
- Reduced transparency.
- Keyboard-only navigation.
- Screen-reader-friendly semantic controls.
- Adjustable notification duration.
- Visual alternatives to sound events.
- Complete mute mode.

## 26. Security UX

Security prompts must clearly identify:

- Application requesting access.
- Resource being accessed.
- Reason for access.
- Allow/deny choices.
- Persistent permission state.

Critical operations require explicit confirmation.

## 27. Developer Mode

Developer Mode exposes:

- VM debugger.
- CPU registers.
- Memory viewer.
- I/O viewer.
- Device inspector.
- Linux logs.
- MSTOS service logs.
- Audio event inspector.
- GUI event inspector.
- CP/M debugger.
- Snapshot manager.
- Performance counters.

## 28. GUI Architecture

```text
+---------------------------------------------------+
| MSTOS Desktop Shell                               |
+---------------------------------------------------+
| Window Manager | Launcher | Notifications        |
+---------------------------------------------------+
| System UI Services                                |
+---------------------------------------------------+
| MSTOS C++23 API                                  |
+---------------------------------------------------+
| Linux User Space / Guest Services                 |
+---------------------------------------------------+
| Linux Kernel                                      |
+---------------------------------------------------+
| Virtual Hardware                                  |
+---------------------------------------------------+
| Windows Host / Future ARM64-RISC-V Hardware      |
+---------------------------------------------------+
```

The GUI must not directly depend on the Windows host. The UI API should target the MSTOS abstraction layer so that the same shell can eventually operate on physical ARM64/RISC-V hardware.

## 29. Recommended Technology

The GUI should be implemented as an original C++23 application layer.

Recommended architecture:

- C++23 core API.
- Platform abstraction layer.
- Linux/Windows rendering backend.
- GPU-accelerated compositor where available.
- Input abstraction.
- Accessibility abstraction.
- Audio event API.
- Theme/token system.
- Application/plugin registry.

The implementation should avoid hard-coding GNOME-specific APIs into MSTOS core interfaces.

## 30. GUI File/Module Plan

```text
src/gui/
  app/
  shell/
  compositor/
  window/
  launcher/
  dock/
  workspace/
  panel/
  notifications/
  settings/
  filemanager/
  terminal/
  systemmonitor/
  vmcontrol/
  cpm/
  recovery/
  accessibility/
  theme/
  icons/
  input/
  audio/
  services/
```

## 31. GUI State Machine

```text
BOOT
 |
v
LOGIN
 |
v
DESKTOP
 |\
 | +--> APPLICATION
 | +--> SETTINGS
 | +--> TERMINAL
 | +--> CP/M
 | +--> VM CONTROL
 | +--> RECOVERY
 |
v
LOGOUT
 |
v
SHUTDOWN / REBOOT
```

## 32. Originality Requirement

MSTOS may use established desktop concepts and documented Linux interfaces, but its implementation, UI layouts, APIs, themes, icons, sound sequences, configuration formats, and application integration should be independently authored for this project.

Third-party components must retain their applicable licenses and attribution requirements.

## 33. Development Roadmap

### GUI Phase 1 — Shell Foundation

- Window abstraction.
- Desktop surface.
- Top bar.
- Launcher.
- Basic window management.

### GUI Phase 2 — Core Applications

- Terminal.
- File manager.
- Settings.
- System monitor.
- VM control panel.

### GUI Phase 3 — System Integration

- Linux service integration.
- Notifications.
- Audio events.
- Storage.
- Networking.
- Power.

### GUI Phase 4 — Legacy Integration

- CP/M launcher.
- Osborne control panel.
- Disk-image manager.
- Z80 debugger.

### GUI Phase 5 — Visual Identity

- Final MSTOS theme.
- Icon set.
- Typography.
- Animations.
- Loading screen.
- Boot graphics.
- Sound theme.

### GUI Phase 6 — Hardware Portability

- ARM64 display backend.
- RISC-V display backend.
- Physical keyboard/input.
- Physical audio.
- Physical storage.
- FPGA/SoC device integration.

## 34. Definition of Done

The redesigned MSTOS GUI is complete when:

- The desktop can boot and operate without GNOME being a required MSTOS dependency.
- Windows can be moved, resized, tiled, minimized, maximized, and closed.
- Applications can be searched and launched.
- Terminal and file manager are functional.
- Settings can configure the major system services.
- VM controls are available.
- CP/M compatibility is accessible from the desktop.
- Boot/loading/recovery states have dedicated UI designs.
- Audio events are mapped to GUI/system events.
- Accessibility controls work.
- The shell can be ported away from the Windows host abstraction.
- The final visual system is recognizably MSTOS rather than a clone of Ubuntu/GNOME.
