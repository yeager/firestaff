# DM2 V1 — Platform/Build Audit: Startup Sequence

## DM2 Startup Sequence

### SKULLWIN Startup (Allegro 5)

From `c_allegro.cpp` and `main.cpp`:

```
1. al_init()                          — Allegro library init
2. al_install_keyboard()
3. al_install_mouse()
4. al_create_timer(0.01)              — 100 Hz game timer
5. al_create_event_queue()
6. al_set_new_display_option(ALLEGRO_COLOR_SIZE, 32, ALLEGRO_REQUIRE)
7. al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE)
8. al_set_new_display_refresh_rate(60)
9. al_create_display(640, 400)        — 2x scaled from 320×200
10. dm2sound.init()                  — audio subsystem
11. Register keyboard + mouse event sources
12. Main loop:
    a. al_wait_for_event(event_queue)
    b. Handle input (keyboard/mouse)
    c. alFlipDisplay()                — vsync at 60 Hz
```

### SKWIN-SDL Startup

From `SkwinSDL.cpp`:

```
1. SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)
2. SDL_SetVideoMode(640, 400, 8, SDL_HWSURFACE | SDL_DOUBLEBUF)  — or scaled
3. glbPaletteRGB[256][3] init
4. vram[65536] = {0}                  — clear VRAM
5. Game state init (SkGlobal.cpp):
   - bUseDM2ExtendedMode = (DM2_EXTENDED_MODE == 1)
   - s_textLangSel[category][index][field] — text localization tables
   - strSKSaveIOBin = "SKSaveIO.bin"  — temp save verification file
6. Load DUNGEON.DAT, GRAPHICS.dat
7. Main loop: SDL_PollEvent → process → SDL_BlitSurface → SDL_Flip
```

### SKWINDOS Startup (DOS)

Real-mode DOS entry:
```
1. _AH = 0x00; int86(0x10)            — set VGA mode 13h (320×200×256)
2. Install interrupt handlers
3. Init PC speaker / Sound Blaster
4. Load data files from CWD or CD
5. Game loop with VSYNC wait (port 0x3DA)
```

### DM2 vs DM1 Startup Differences

| Step              | DM1                    | DM2                           |
|-------------------|------------------------|-------------------------------|
| Display init      | SDL_SetVideoMode 320×200 | 320×200→640×400 2x scale      |
| Timer             | SDL timer or custom    | Allegro timer at 100 Hz       |
| Data dir scanning | Absolute path to data  | Hash-based asset_find_by_md5  |
| Dungeon loading   | dm1_load_dungeon()     | dm2_v1_load_dungeon()          |
| Outdoor support   | None (indoor only)     | is_outdoor() check + renderer |
| Language support  | None                   | s_textLangSel (GDAT categories)|
| Sound             | DM1 audio (Allegro/SDL)| HMP MIDI + Allegro audio       |
| Save format       | M10 save format        | SKSAVE*.DAT (different header) |
| Extensions        | None                   | DM2_EXTENDED_MODE preprocessor |

### Firestaff DM2 V1 Init

In `dm2_v1_game.c`:

```c
void dm2_v1_init(DM2_V1_GameState *state, const char *data_dir) {
    memset(state, 0, sizeof(*state));
    state->data_dir = data_dir;
    state->party_x = 15;
    state->party_y = 15;
    state->party_dir = 0;
    state->gold = 100;
    state->time_of_day = 720;  // 720 minutes = noon
}
```

Startup sequence in Firestaff:
1. `dm2_v1_init()` — set defaults
2. `dm2_v1_load_dungeon()` — hash-search for DUNGEON.DAT
3. `dm2_v1_dungeon_load()` — parse level table from raw bytes
4. (Future: outdoor renderer init via `dm2_v1_outdoor_init()`)

### Dungeon Load Flow

```
dm2_v1_load_dungeon(state)
  → asset_find_by_md5_list(data_dir, dm2_dungeon_hashes[], path)
    → if found: dm2_v1_dungeon_load(out, dat, size)
      → rd16(dat+0) = level_count
      → for each level: rd16(dat+offset) = level_type, width, height, offset
      → store raw_data (malloc'd)
    → if not found: return -1 (dungeon files in zip archives)
```

DM2 dungeon files need extraction from zip archives before loading.
DM1 dungeon files are typically loose on disk and hash-found directly.

### Save Game Load

`dm2_v1_save_load.c` handles SKSAVE*.DAT:
- `dm2_v1_save_game(path, state, size)` — serialize to disk
- `dm2_v1_load_game(path, state, max_size)` — deserialize
- Source: SKULL.ASM save/load routines
