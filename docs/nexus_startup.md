# Nexus V1 Startup Sequence Audit — Source-Locked

## Sources
- `src/nexus/nexus_v1_engine.c` (init, main loop, ISO detection)
- `src/nexus/nexus_v1_game.c` (game state init)
- `include/nexus_v1_engine.h`
- `docs/nexus_overview.md`, `docs/platform/platform_startup.md`
- `docs/nexus_menus.md`, `docs/nexus_testing.md`

---

## 1. Startup Overview

### Platform Context
**Original Saturn boot:**
- Boot from CD-ROM (Track 1 = ISO 9660 game data)
- SH-2 CPUs initialize, VDP1/VDP2 configured
- Game executable runs from CD (no installation)
- No save data on cartridge — SRAM cartridge save (8 KB)

**Firestaff PC startup:**
- Binary invoked from command line / desktop
- `cmake` + `cmake --build` produces executable
- Data directory passed as argument or discovered via search roots

---

## 2. Nexus V1 Initialization Sequence

### Engine Init: nexus_v1_init()
Located in `src/nexus/nexus_v1_engine.c`:

```c
int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir)
```

**Step 1 — Platform detection and ISO search:**
```c
// Priority 1: Look for .cue file in data directory → open .bin as ISO
if (find_iso(data_dir, cue_path, sizeof(cue_path))) {
    int n = nexus_iso_open_cue(&engine->iso, cue_path);
    if (n > 0 && nexus_iso_is_nexus(&engine->iso)) {
        engine->source = NEXUS_SRC_ISO;
    }
}

// Priority 2: Look for extracted files on disk
if (engine->source == NEXUS_SRC_NONE && has_extracted(data_dir)) {
    engine->source = NEXUS_SRC_EXTRACTED;
}
```

The engine supports two data source modes:
- `NEXUS_SRC_ISO`: Read directly from CUE/BIN disc image
- `NEXUS_SRC_EXTRACTED`: Read from extracted CD contents

**Step 2 — Game state init:**
```c
nexus_v1_game_init(&engine->game, data_dir);
engine->audio_enabled = 1;
```

**Step 3 — Font loading:**
```c
uint8_t *font_data = nexus_v1_read_file(engine, "FONT256.S2D", &font_size);
if (font_data) {
    engine->font_loaded = (nexus_v1_font_load(&engine->font, font_data, font_size) > 0);
    free(font_data);
}
```

**Step 4 — Mark initialized:**
```c
engine->initialized = 1;
```

---

## 3. Data Source Discovery

### ISO Disc Image Discovery
`find_iso()` function in `nexus_v1_engine.c`:

**Windows** (`_WIN32`):
```c
WIN32_FIND_DATAA fd;
HANDLE h = FindFirstFileA(pattern, &fd);  // *.cue pattern
```

**POSIX** (Linux/macOS):
```c
DIR *d = opendir(dir);
struct dirent *ent;
// scan for *.cue file
```

### Extracted Files Discovery
`has_extracted()` checks for these sentinel files:
- `DM.BIN` (disc image data archive)
- `LEV00.DGN` (level 0 dungeon file)

Either sentinel existence triggers extracted mode.

### Data Directory Search Roots
Same priority as DM1 (platform_os.md):
1. Explicit `requestedDir` argument (if non-empty)
2. `FIRESTAFF_DATA` environment variable
3. Platform-specific user data dir:
   - Windows: `%APPDATA%\Firestaff`
   - macOS: `~/Library/Application Support/Firestaff`
   - Linux: `~/.local/share/Firestaff`
4. Current directory `.`

---

## 4. Game State Initialization

### nexus_v1_game_init()
`src/nexus/nexus_v1_game.c`:
- Initializes champion roster (24 Japanese champions)
- Sets up inventory slots, spell book, experience tracking
- Initializes dungeon state (no level loaded yet)
- Sets party position to entry point of current level

### Champion Roster (Japanese)
Unlike DM1's Western names (Thor, Sara…), Nexus champions have Japanese names:
- **Syra, Leyla, Nabi…** (examples from nexus_overview.md)
- 24 champions total (same count as DM1)
- Same advancement mechanics (XP, levels, stats)

---

## 5. Level Loading

### nexus_v1_load_level()
Called when entering a dungeon level (0-15):
```c
int nexus_v1_load_level(Nexus_V1_Engine *engine, int level)
```

Loads:
1. **LEV__.DGN** file (dungeon grid + 3D geometry)
2. **SNDLEV__.SAL** sound bank (290-460 KB per level)
3. **SLEV__.BIN** script file (2-12 KB per level)
4. **SMAP__.BIN** minimap data (17-30 KB per level)

### Sound Bank Loading
On level load:
- Load appropriate SNDLEV file
- Start CD audio track for level (track = level + 1)
- ADX/SEGA PCM SFX loaded from sound bank

---

## 6. Main Loop Structure

### Game Tick Loop
`nexus_v1_engine.c` runs a tick-based loop:
```
Input poll → Command queue → Sensor/trigger →
Movement → Combat → AI → Dungeon events → Viewport render
```

DM1 mechanics underneath — same tick order as DM1 V1 engine.

### Viewport Render
Each tick:
1. `nexus_v1_viewport_render()` draws first-person 3D view
2. Software rasterizer (nexus_v1_rasterizer.c) → 320×200 framebuffer
3. DMA upload to display (Saturn VDP1) / SDL present (PC)

### CD Audio Track Management
Track numbers on disc:
- Track 1: Game data (ISO)
- Tracks 2-9: Red Book Audio CD-DA (8 music tracks)
- Track for level N: track (N+2) — level 0 uses track 2, level 1 uses track 3, etc.

---

## 7. Menu / UI Startup

### Startup Menu Sequence
Based on `docs/nexus_menus.md` and DM1 startup pattern:
1. **Title screen** — 3D rendered or pre-rendered graphic
2. **Champion selection** — Hall of Champions (24 JP champions)
3. **Dungeon entrance** — 3D entrance scene
4. **In-game UI** — Champion panels, inventory, spell list

### Font Rendering
`nexus_v1_saturn_font.c` handles Japanese Shift-JIS text:
- 256-character font including Japanese
- Shift-JIS encoding for Japanese text in menus/inscriptions
- Rendered via `nexus_v1_text_draw()` functions

---

## 8. Original Saturn Boot Sequence

### CD-ROM Boot (Original Hardware)
1. Saturn BIOS reads Track 1 (ISO 9660)
2. Locate and execute `SYSTEM.CNF` (Saturn config file)
3. SH-2 CPUs initialize at provided entry point
4. VDP1/VDP2 configured (320×224 NTSC or 320×240 PAL)
5. Game data loaded from CD into work RAM (2 MB)
6. Sound system initialized (ADX streamer ready)
7. SRAM save data loaded (if present)
8. Title screen displayed

### System Configuration File
`SYSTEM.CNF` on Saturn CD:
```
Boot = NEXUS.BIN
CD drive = 2
Region = J
...
```

### SRAM Save
- 8 KB SRAM on cartridge
- Stores champion progress, dungeon state, settings
- Same format as DM1/CSB save games

---

## 9. PC Firestaff Startup (Comparison)

| Step | Original Saturn | Firestaff PC |
|------|-----------------|--------------|
| Boot | CD-ROM bootstrap | CLI binary exec |
| Data loading | CD DMA to RAM | File read from disk/ISO |
| Memory | 2 MB SH-2 RAM | Virtual memory (GB) |
| Display init | VDP1/VDP2 config | SDL_CreateWindow |
| Font | FONT256.S2D from CD | Same file from data dir |
| Entry point | `nexus_v1_init()` | Same function |
| Main loop | SH-2 tick loop | while(!quit) poll events |

---

## 10. Startup Problems / Known Gaps

**In Firestaff Nexus implementation:**
- DGN 3D geometry blob parser: NOT YET IMPLEMENTED
- SDDRVS.TSK script VM: NOT YET IMPLEMENTED
- Sound bank (SAL) parser: NOT YET IMPLEMENTED
- CD audio track playback: NOT YET IMPLEMENTED
- No executable links against `firestaff_nexus` library

The engine init succeeds for grid parsing but full 3D rendering
requires the unimplemented geometry blob parser.
