# DM2 V1 Source Structure

## Module Hierarchy

The codebase is organized in layers from low-level hardware/BIOS emulation up to game logic and UI.

Layer 1 — Hardware/Platform Abstraction
- ints.h/cpp         — Interrupt vectors, int 08h (timer), int 09h (keyboard), int 33h (mouse)
- emu.h/cpp         — x86 emulation helpers, port I/O
- regs.h            — CPU register state structures

Layer 2 — Core Systems
- SkWinCore.h/cpp   — GAME_LOOP(), FIRE_MAIN(), FIRE_BOOTSTRAP(), all game subsystems
- SkWin.h/cpp       — Input queue (mouse + keyboard buffers, CSkWin class)
- SkGlobal.h/cpp    — Global variables (glbChampionTable, fileHeader, fset, etc.)
- DME.h/cpp         — Common code shared across modules (not tied to a class)

Layer 3 — Graphics Pipeline
- c_gfx_main.cpp/h  — c_gfx_system (dm2screen[320x200], backbuffer, pictbuff)
- c_gfx_blit.cpp/h  — blit_toscreen(), BLITMODE0, alpha masking
- c_gfx_bmp.cpp/h   — BMP header parsing, bitmap loading
- c_gfx_decode.cpp/h— Image/compression decoding
- c_gfx_pal.cpp/h   — Palette management (paldat)
- c_gfx_pixel.cpp/h — Pixel format conversion
- c_gfx_str.cpp/h   — String/glyph rendering

Layer 4 — UI/Overlay
- c_gui_draw.cpp/h  — GUI drawing routines
- c_gui_vp.cpp/h    — Viewport management
- c_buttongroup     — Button widget system
- c_buttons.cpp/h   — Button logic
- c_clickrect.cpp/h — Clickable rectangle regions

Layer 5 — Gameplay
- c_hero.cpp/h      — Champion/party member management
- c_creature.cpp/h  — Monster/NPC definitions
- c_ai.cpp/h        — Creature AI, engagement decisions
- c_engage.cpp/h    — Combat engagement system
- c_item.cpp/h      — Item/object management
- c_map.cpp/h       — Dungeon map loading/display
- c_move.cpp/h      — Movement and pathfinding
- c_moverec.cpp/h   — Movement recording and playback
- c_cloud.cpp/h     — Cloud/fog of war effect
- c_light.cpp/h     — Lighting per tile/champion
- c_dialog.cpp/h    — In-game dialogue

Layer 6 — Data/File I/O
- c_gdatfile.cpp/h  — Graphics .DAT file access (img1.dat, mouse1/2.dat)
- c_image.cpp/h     — Image resource loading
- c_savegame.cpp/h  — Save/load game state
- fileio.cpp/h     — File operations

Layer 7 — Audio
- c_midi.cpp/h      — MIDI music playback
- c_sound.cpp/h     — Sound effect system
- c_sfx.cpp/h       — Sound effects
- c_music_wav.cpp/h — WAV audio playback

Layer 8 — System Services
- c_eventqueue.cpp/h — Event queuing
- c_events.cpp/h    — Event processing
- c_timer.cpp/h     — Timer subsystem
- c_tim_proc.cpp/h  — Timed procedures
- c_alloc.cpp/h     — Memory allocator
- c_dballoc.cpp/h   — Debug allocator
- c_random.cpp/h    — RNG
- c_record.cpp/h    — Recording/playback
- c_weather.cpp/h   — Weather effects

## Key Entry Points

- FIRE_BOOTSTRAP()  — Application bootstrap, calls FIRE_MAIN
- FIRE_MAIN()       — Entry point after bootstrap, processes command line args
- GAME_LOOP()       — Core game loop (per-frame update + render)
- IBMIO_MAIN()      — Low-level IBM PC BIOS/DOS compatibility layer entry
- SkWinCore::*      — 256 interrupt vector table (_intrvect[256])

## File I/O and Data Flow

- fileSet fset       — Master file index for all game assets
- c_gdatfile         — Reads img1.dat (graphics), mouse1/2.dat
- c_savegame         — Handles .sav files (champion state, dungeon state)
- c_loadlevel        — Loads dungeon levels from asset files

## SKWIN (MFC) Specific

Adds MFC UI layer on top of SkWinCore:
- SkWinMFC.cpp/h    — MFC message map and window frame
- SkWinMIDI.cpp/h   — MIDI music (Windows MCI API)
- SkwinSDL.cpp/h    — SDL wrapper for rendering
- SkVersionControl.h — Version stamps for build variants
- SBlast.cpp/h      — Sound Blaster detection/initialization
