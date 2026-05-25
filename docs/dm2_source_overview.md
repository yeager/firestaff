# DM2 V1 Source Overview

## Repository

Source: https://github.com/gbsphenx/skproject
Local canonical anchor: /home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm2/SKULL.ASM

## Original Disassembly

**SKULL.ASM** — 522,128 lines, single-segment 16-bit x86 DOS binary

Disassembly of the DOS SKULL.EXE (Dungeon Master II: Skullkeep). The entire game binary is contained in one seg000 segment. Organized by address ranges: sub_100_xxx (boot/intro), sub_200_xxx through sub_10xx_xxx (game logic). The MZ header starts at lines 78-113.

## skproject Directory Structure

skproject/
├── SKULLWIN/     Allegro 4.2 C++ port from PC-DOS base (66 .cpp / 65 .h)
├── SKWIN/        MFC Windows port from PC-9821 (15 .cpp)
├── SKWINDOS/     DOS SDL/Allegro port from PC-DOS (15 .cpp)
├── SKWINSPX/     New unified source for Windows + Linux + DOS (in progress)
├── DMDC2/        Dungeon map editor
├── DM2GDED/      Graphics.dat editor
├── Utils/        Utilities
└── installer/    Installation scripts

## SKULLWIN Classes (66 C++ pairs)

Graphics: c_gfx_main, c_gfx_blit, c_gfx_bmp, c_gfx_decode, c_gfx_pal, c_gfx_pixel, c_gfx_str
UI: c_gui_draw, c_gui_vp, c_buttongroup, c_buttons, c_clickrect, c_dialog
Gameplay: c_ai, c_creature, c_hero, c_item, c_map, c_move, c_moverec, c_engage, c_cloud, c_light
System: c_eventqueue, c_events, c_input, c_keybd, c_mcursor, c_midi, c_loadlevel, c_addon
Data: c_gdatfile, c_image, c_savegame, c_sound, c_sfx, c_music_wav, c_record, c_weather
Core: c_alloc, c_dballoc, c_random, c_rect, c_str, c_timer, c_tim_proc, c_tmouse, c_xrect, c_querydb
DME: Common code outside classes (DME.cpp/h)
SkWin: Core input/mouse/keyboard queue class
SkWinCore: Main windowing/game loop class (GAME_LOOP, FIRE_MAIN, FIRE_BOOTSTRAP)
SkGlobal: Global state variables
SkCodeParam: Configuration and parameters

## SKWIN Module (MFC Windows, 15 .cpp)

Full MFC application. Exe variants: Standard, Extended, SuperMode, Mod.Release, Release_EN_JP, Release_JP_JP.
Key classes: SkWinCore (GAME_LOOP, FIRE_MAIN), SkWinMFC, SkWinMIDI, SkWinDebug, SkwinSDL.

## SKWINSPX (New Unified Code)

Consolidates SKWIN and SKULL into one source tree for Windows + Linux + DOS.
Build systems: makefile_linux_*, makefile_mingw_*, XBUILDV*.BAT.
Directories: src/ (with v0/v4/v5/v6 sub-versions), documentation/.

## Source Lineage

- SKULLWIN/SKWINDOS: Direct C++ port of PC-DOS SKULL.EXE disassembly
- SKWIN: Ported from PC-9821 version (v4/v5), MFC UI layer
- SKWINSPX: New unified effort merging all branches
- Original: SKULL.ASM disassembly (522K lines, single 64KB seg000)
