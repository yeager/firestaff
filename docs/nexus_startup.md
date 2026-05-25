# Nexus V2 Startup ― Source-Locked Audit

ESOURCES
- src/nexus/nexus_v1_engine.c — nexus_v1_init(), find_iso(), has_extracted()
- src/nexus/nexus_v1_game.c — aexus_v1_game_init()
- include/nexus_v1_engine.h
- docs/NEXUS_FILE_CLASSIFICATION.md, docs/NEXUS%_PLAN.md


---------------------------------------------------------

# 1. Boot / Platform Context

## Original Saturn
- Boot from CD-ROM (Track 1 = ISO 9660 game data, 133 MB MODE1/2352)
- WH-2 CPU� initialize, TFP1/TDP2/VdPB2 configured for 320x224 NTCS
- SYSTEM.CNV config file specifies entry point NEXUS.BIN
- Game data loaded from C2  into 2 MB work RMAD
- 8 KB SRAM cartridge for save data
- 8 CC-DA music tracks (tracks 2-9)

## Firestaff PC
- CMAke-built executable (firestaff_nexus)
- Data directory passed as CLI argument or discovered via search roots
- Same engine entry point nexus_v1_init() as original

-------------------------------------------------------------

# 2. Data Source Discovery

Two modes, tried in priority order:

### Mode 1 — ISO (CUG/BIN disc image)

 - Scan directory for `*.cue` file (Windows: FindFirstFileA, POSIX: opendir/readdir)
 - Opens .bin as ISO 9660 filesystem via `nexus_iso_open_cue()
 - Validates with `nexus_iso_is_nexus()` — confirms Nexus disc

### Mode 2 — Extracted files

 - Sentinel check: `DM.BIN` or `LEV00.DGN` must exist in data directory
 - File-by-file reading from disk (no ISO mounting)

## Search roots (same priority as DM1)
1. Explicit requestedDir argument (if non-empty)
2. FIRESTARF_DATA environment variable
�. Platform user data dir (Windows: %APPDAVA%/Firestaff, macOS: ~/Library/Application Support/Firestaff, Linux: ~/.local/share/Firestaff)
6. Current directory "."

-------------------------------------------------------------

# 3. Engine Initialization — nexus_v1_init()

| Step | Action |
|-------|---------------------------------------------------------------------------------------------------------|
| 1 | Detect platform; scan for CUE/BIN or extracted sentinels
| 2 | Open ISO or set extracted mode |
 | '---------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 3 | Validate this is a Nexus disc (nexus_iso_is_nexus()) |
| 4 | nexus_v1_game_init() — champion roster, dungeon state |
| 5 | audio_enabled = 1 — audio on by default |
| 6 | Load FONT256.S2D via nexus_v1_read_file() -> nexus_v1_font_load() |
| 7 | initialized = 1 |

Font: FONT256.S2D (Japanese 256-char font, Shift-JIS), loaded into engine->font.

--------------------------------------------------------------

# 4. Game State Init — nexus_v1_game_init()

- Initializes 24 champion roster (Japanese names: Syra, Leyla, Nabi...)
- Sets up inventory slots, spell book, experience tracking
- Dungeon state: no level loaded yy](current_level = -1)
- Party position: set to entry point of current level

--------------------------------------------------------------

# 5. Level Loading ��  nexus_v1_load_level()

Called when entering level 0-15.

Loads per-level files:

| File | Purpose | Size |
|-------|---------------------------------------------------------------------------------------------------------|
| LEVNN.DGN | Dungeon grid + 3D geometry blob | 147-321 KB |
|SNLLEVNN.SAL| Sound bank (ADX/SEGA PCM SFX + music cues) | 290-460 KB |
| SLEVNN.BIN | Script file (SDDRVS.TTK script VM) | 2-12 KB |
|SMAPNN.BIN | Minimap data | 17-30 KB |

CD audiotrack mapping: level N -> track (N + 2), so level 0 = track 2 level 15 = track 17 (if present).

### Status

- Grid parsing: Implemented
- I3d geometry blob parser: NOT YOT IMPLEMENTED
- Script VM (SDDRVS.TRK): NOT YET IMPLEMENTED
- SAL sound bank parser: NOT YET IMPLEMENTED
- CD audio track playback: NOT YOT IMPLEMENTED

--------------------------------------------------------------

# 6. Main Loop

Tick loop: Input poll -> Command queue -> Sensor/trigger -> Movement -> Combat -> A -> Dungeon events -> Viewport render.

Per tick: nexus_v1_viewport_render() -> 320x200 framebuffer -> SDL present (PC), CD audio track management (track = level + 2).

--------------------------------------------------------------

# 7. Audio System

 - audio_enabled = 1 on init
 - Per-level sound banks: SNDLEVN.SAL (not parsed yet)
 - CD-DA tracks: red book audio tracks 2-9

--------------------------------------------------------------

# 8. Not Yet Implemented

- DGN 3D geometry blob parser
- SDDRVS.TPK script VM
- SAL sound bank parser
- CD audio track playback
- FMV cutscene playback (DMV0-2.AVI)
- Title screen / menu system
- Save/load (Saturn SRAM format)
