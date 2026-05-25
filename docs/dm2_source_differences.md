# DM2 vs DM1: Key Differences

## Bit Depth

DM1: 8-bit graphics, 8086/8088 real-mode, tight 64KB segment
DM2: Same 320x200x256 VGA base, but extended with near/far pointer model
- Uses 16-bit segment:offset addressing in original DOS binary
- C++ port uses flat 32-bit pointers via Allegro 4.2

## UI and Rendering

DM1: Pure raycasting 3D, fixed viewport (no scrolling), simple status line
DM2: More complex UI with:
- Multi-region display (3D view + side panels + action bar)
- c_gui_draw and c_gui_vp for complex UI compositing
- Double-buffered rendering with backbuffer
- c_cloud for fog of war per tile
- c_moverec for movement recording/playback
- c_weather for weather effects
- Champion portraits and detailed stat panels (c_hero)

## Dungeon System

DM1: Single dungeon, 4 party members max
DM2: Larger dungeon with:
- c_map: tilemap with multiple floor levels
- c_cloud: fog of war (tiles hidden until explored)
- More sophisticated lighting model (c_light per tile)
- Party members with individual inventories (4 champions, 3 slots each visible)
- Champions can be disabled/defeated without game over (resurrection timers)

## Combat

DM1: Simple melee/ranged, one attack per round
DM2: More complex via c_engage and c_ai:
- Engagement state tracking
- Creature AI with multiple behaviors
- PROCESS_PLAYERS_DAMAGE — distributed damage model
- Champion-specific effects (burning items, torch light decay)

## Memory and Storage

DM1: 64KB segment, saves to single file with tight encoding
DM2: Extended model, savegames via c_savegame:
- Champion state: stats, HP, inventory, position, effects
- Dungeon state: explored tiles, creature positions, item placements
- FileSet index for accessing multiple .DAT resources

## Audio

DM1: PC speaker beeps (simple frequency/duration)
DM2: Full MIDI music (c_midi/SkWinMIDI) + WAV sound effects (c_sound/c_sfx)
- PROCESS_SOUND for ambient sound
- c_music_wav for additional audio
- SBlast.cpp for Sound Blaster detection

## Development Toolchain

DM1: Pure hand-written 8086 assembly (single file, ~45K lines disassembly)
DM2: Disassembly still 522K lines, but C++ ports exist:
- SKULLWIN: Allegro 4.2 C++ (full class-based rewrite)
- SKWIN: MFC C++ (PC-9821 port)
- SKWINSPX: Unified multi-platform build (Windows/Linux/DOS)

## Windows-Specific Code

SKWIN introduces Windows-specific layers:
- SkWinMFC: MFC message pump, window management
- SkwinSDL: SDL wrapper for cross-platform rendering
- SkWinMIDI: Windows MCI API for MIDI playback
- IBMIO_* functions: BIOS-level emulation of DOS compatibility layer

## Version Variants

SKWIN ships 6 compiled .exe variants:
- Standard, Extended, SuperMode, Mod.Release
- Release_EN_JP (English/Japanese), Release_JP_JP (Japanese)
- Different config/language/content combinations

SKWINSPX has v0, v4, v5, v6 source subdirectories (corresponding to PC-9821 DM2 versions v0 through v5).

## New Classes in DM2 (Not in DM1)

c_cloud — fog of war
c_moverec — movement recording
c_weather — weather effects
c_music_wav — WAV audio playback
c_record — session recording
c_savegame — save/load game
c_tim_proc — timed procedures
c_tmouse — tracked mouse
c_xrect — extended rectangles
c_querydb — query database
c_dballoc — debug memory allocator

## Constants

DM2 backbuffer: 224 x 136 (vs DM1 320 x 136 viewport)
DM2 uses ORIG_SWIDTH=320, ORIG_SHEIGHT=200
Timer interrupt: int 08h (same as DM1 for BIOS compatibility)
