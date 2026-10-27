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

Firestaff exposes two player-facing presentation choices. **Original (v1)**
is the preservation route, while **Modern (v2.x)** enables optional native
visual improvements. Aspect ratio and output resolution are available in both
modes. They affect only the host presentation target: the renderer fits the
native source frame with bars where necessary and never stretches it
non-uniformly.

## Game Cards

Selecting a game card opens a three-step launch flow. First, Firestaff shows
generated cards for each natively supported platform. Each card states whether
the scanner verified matching game data; a platform without verified data is
visible but cannot be started. The initial selection preserves an explicit
CLI or saved platform preference; automatic selection chooses the first
verified platform.

After a verified platform is selected, **Original** starts the preservation
preset and **Modern** starts the v2.x preset directly. **Custom** opens the
detailed options page for per-setting changes. Game archives remain the launch
source throughout this flow; Firestaff does not extract a copy to disk.

All launcher menus support mouse navigation. Cards, settings controls, Museum
categories and their visible action buttons have exact click targets. In
document and list views, clicking the left or right third changes the page or
category, clicking the upper or lower centre scrolls, and clicking the centre
activates the current item. The Back button is available in every non-main
view. Empty platform-picker cells are inert; only a shown platform card can be
selected, and a card without verified game data remains unable to launch.

M12_SETTINGS_TAB_GAME       = game settings
M12_SETTINGS_TAB_GRAPHICS  = renderer, scale mode, aspect ratio
M12_SETTINGS_TAB_CONTROLS  = input remapping, WASD, touch
M12_SETTINGS_TAB_AUDIO     = master/music/SFX volume, mute
M12_SETTINGS_TAB_ACCESSIBILITY = font scale, high contrast, colorblind mode

The high-contrast row toggles both the launcher chrome (existing
`m12_presented_color` in `src/ui/menu_startup_m12.c`) and the M11
in-game chrome via the new `m11_high_contrast_overlay_pc34_compat`
gate (`M11_HighContrast_SetActive()` is pushed from
`m12_apply_loaded_config` next to the existing
`M11_UIScale_SetPercent` push). The gate covers HUD text, dialog
choice text, action-area / rune-strip text, combat-log text, hit-
flash text, and the death / winner overlay. The 320x200 dungeon-
viewport pixels and the HUD panel C040 / C017 backdrop blits are
deliberately preserved bit-exact because they go through separate
bitmap-blit paths, not through `m11_draw_text`. Default state is
off so V1 launches stay pixel-identical with the original DM1 PC
3.4 presentation.

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
# Live runtime graphics

The per-game **Game** settings in the start menu contain the initial
presentation mode, cheat switch and speed. The same values can be changed
after launch with **F10**. F10 writes the selected game's configuration slot
immediately, so the setting is retained for the next launch.

F10 is supported by DM1, CSB, DM2, Theron's Quest and Nexus. Presentation
changes affect the active renderer in real time; no restart is required.
