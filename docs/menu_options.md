# DM1 V1 - Options / Settings

## Source-Locked
ReDMCSB WIP20210206 - Toolchains/Common/Source/

---

## Appendix/Options System (APPA.C)

APPA.C is the FTL (Folded Temporary Library) application shell. It handles
loading and sequencing of ancillary modules:

- FTL_TITL  = title/intro animation module
- FTL_ANIM  = animation module
- FTL_APPB  = appendix/options module (invoked with FTL_APPB parameter)
- FTL_ENDA  = end credits module
- FTL_KAOS  = language/font module
- FTL_SWSH  = software switch module (hardware detection)

The main function (APPA.C:26) sequences these based on L2157_s_.Parameter.

DOS PC98 display mode selection (DMSTART.C:164-210):
- G2334_apc_SelectGraphicModeMessage[24] - display mode options
- User presses 1 or 2 to select (DMSTART.C:207)
- Result stored in G8253_ (DMSTART.C:175)

Monitor types: PC9801LV51C RAM at F0000 or E0000, VX/RX/RA/RS, UX/EX/ES/CV, UV21/DO

---

## Sound / Music (STARTUP2.C / SOUND.C)

Sound initialization: F0062_SOUND_Initialize (STARTUP1.C:160, STARTUP2.C:1344)

Volume struct SOUND_VOLUME: G6196_.Left and G6196_.Right (ENTRANCE.C:186-187)
Default: 255/255 (full volume). No runtime mixer UI in DOS version.

Music:
- F0741_MUSIC_PlayGameMusic(C0_MUSIC_ENTRANCE) - entrance music (ENTRANCE.C:836)
- F0814_TRansition_MIDIMusic() - crossfades to gameplay music (F31E/F31J)

Sound effects: F0064_SOUND_RequestPlay_CPSD for spatial combat feedback.

---

## Mouse / Sensitivity

Mouse cursor bounds set in ProcessEntrance:
- F0785_SetMousePointerCoordinates(251, 51) on F20E (ENTRANCE.C:832)
- F0785_SetMousePointerCoordinates(251, 49) on F20J/P20JA/P20JB (ENTRANCE.C:834)

---

## Palette / Brightness Control

F0694_SetMultipleColorsInPalette sets palette presets (ENTRANCE.C:434):
- C10_ENTRANCE_BLACK
- C07_ENTRANCE_DM
- C28_ENTRANCE_CSB

VDEO_14_SetNormalColorPalette / VDEO_15_ApplyNormalColorPalette for
platform-specific palette control (APPA.C:116-119).

---

## Display / Graphics Options (Firestaff Modern UI)

menu_startup_m12.c provides the full options panel that DM1 V1 lacked:

M12_SETTINGS_TAB_GAME       = game settings
M12_SETTINGS_TAB_GRAPHICS  = renderer, scale mode, aspect ratio
M12_SETTINGS_TAB_CONTROLS  = input remapping, WASD, touch
M12_SETTINGS_TAB_AUDIO     = master/music/SFX volume, mute
M12_SETTINGS_TAB_ACCESSIBILITY = font scale, high contrast, colorblind mode

Settings rows include: language, window mode, integer scaling, VSYNC,
viewport style, auto-pause, theme, background.

Legacy DM1 V1 had no in-game display options UI. Settings were platform-
specific and set pre-game via APPA/FTL subsystem.

---

## Summary

DM1 V1 options: minimal. No runtime mixer. Display settings are
platform-specific text prompts at startup. The Firestaff modern UI
(menu_startup_m12.c) provides the full options panel: graphics renderer,
scale mode, audio channels, viewport style, input remapping,
accessibility options.
