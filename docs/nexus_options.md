# Nexus V1 Options Menu — Source-Locked Audit

## Sources
- docs/nexus_menus.md (menu system status)
- src/nexus/nexus_v1_engine.c
- docs/NEXUS_PLAN.md
- src/frontend/ (DM1 options reference)

---

## 1. Options System Status

**NOT YET IMPLEMENTED.**

No options menu exists in the Firestaff Nexus codebase. No
nexus_v1_options.c, no options state machine, no settings persistence.

The options system must be designed based on what is known about the
Nexus engine and what the Saturn platform supports.

---

## 2. Expected Options Categories

### Display Options
- **Resolution**: The original Saturn outputs 320x224 (NTSC) or 320x240 (PAL).
  On PC, this could be upscaled to 640x480, 800x600, 1024x768, or
  rendered at native resolution with aspect ratio preservation.
- **Fullscreen / Windowed**: Standard toggle.
- **Scanlines**: CRT emulation option (off by default).
- **FPS cap**: Lock to 30 FPS (Saturn native) or unlimited.

### Audio Options
- **Music volume**: 0-100, default 70.
- **SFX volume**: 0-100, default 80.
- **CD audio mute**: Toggle for red book audio tracks 2-9.
- **Audio enabled**: Global toggle (engine->audio_enabled).

### Gameplay Options
- **Auto-map**: Show/hide minimap (SMAPNN.BIN data).
- **Auto-save**: Periodic save to SRAM (not implemented).
- **Text speed**: How fast message text scrolls.
- **Battle speed**: Animation speed in combat.

### Language Options
- **Text encoding**: Shift-JIS for Japanese text (default).
- **No language switching**: Nexus is JP-only, no i18n support.

---

## 3. Settings Persistence

### Original Saturn
- 8 KB SRAM cartridge stores:
  - Champion progress
  - Dungeon state
  - Game settings
- SRAM format: binary, 8 KB total

### Firestaff PC
- Settings file: ~/.local/share/Firestaff/nexus_settings.json (or similar)
- Save format: JSON key-value pairs
- No SRAM emulation needed for settings (only for game saves)

---

## 4. DM1 Options Reference

DM1 options (from src/frontend/menu_startup.md reference):
- Sound volume
- Music on/off
- Text speed
- keystroke repeat rate

These are basic by modern standards. Nexus options would extend this
with display mode, fullscreen, and CD audio controls.

---

## 5. Engine-Level Settings

```c
// in Nexus_V1_Engine struct
int audio_enabled;        // 0 or 1, default 1
int music_volume;         // 0-100, default 70
int sfx_volume;          // 0-100, default 80
int display_mode;        // 0=windowed, 1=fullscreen
int upscaling;           // 0=none, 1=2x, 2=3x
int scanlines;           // 0=off, 1=on
```

Options menu would be a state in the nexus_v1_menu_state_t enum,
accessible from both the title screen and the in-game ESC menu.

---

## 6. Not Yet Implemented

- Options menu UI (any screen)
- Settings file read/write
- SRAM save format (for game saves, not just settings)
- Audio volume controls (no audio playback yet)
- Display mode switching
- All options listed above

---

## 7. Next Steps

1. Define Nexus_V1_Settings struct in include/nexus_v1_engine.h
2. Implement nexus_v1_settings_load() / nexus_v1_settings_save()
3. Build nexus_v1_options_menu_render() and nexus_v1_options_menu_input()
4. Add options state to menu state machine
5. Wire volume controls into audio system (once SAL parser exists)