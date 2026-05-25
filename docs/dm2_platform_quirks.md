# DM2 V1 — Platform/Build Audit: Technical Quirks & Limitations

## DM2-Specific Technical Quirks

### 1. Dungeon Files Inside Zip Archives

DM2's DUNGEON.DAT files are often distributed inside zip archives, unlike DM1
where they are loose files on disk. The current `dm2_v1_load_dungeon()` returns
an error until extraction is implemented:

```c
printf("DM2: no verified dungeon hash available yet — "
       "dungeon files need to be extracted from zip archives first\n");
return -1;
```

This is the primary blocker for DM2 V1 runtime parity with DM1.

**Workaround**: Manually extract zip archives to expose loose DUNGEON.DAT files,
then use hash-based discovery.

### 2. Multiple Conflicting Data Variants

The skproject contains ~8 distinct dungeon/graphics dataset variants:

- `data_dm2_dm/` — DM2 Dungeon Master PC English
- `data_dm2_tq/` — Theron's Quest PC variant
- `data_dm2_sk/` — Skullkeep (PC-9821 Japanese + PC DOS)
- `data_dm2_beta/` — Beta pre-release
- `data_dm2_demo/` — Demo version
- `data_dm2_csb/` — Cross-variant (CSB/DM2 mix)
- `data_beta/` — General beta (DUNGEON.DAT, graphics.dat)
- Plus PC-9821-specific variants (PC9821, PC9821_MULTI, PC9821_v4.0.1, PC9821_v4.0d, PC9821_v4.5)

These variants are NOT interchangeable. The code must select the correct
variant for the active dungeon selection (controlled by `SkCodeParam::dung`
in SKWIN).

### 3. GDAT Localized Text Array Size

DM2 uses a 3D text localization table:
```cpp
U8 s_textLangSel[GDAT_CATEGORY_LIMIT][0xFF][0xFF];
U8 s_imageLangSel[GDAT_CATEGORY_LIMIT][0xFF][0xFF];  // DM2_EXTENDED_MODE only
```
This is a 256×256×N byte table — potentially very large in memory.
The index `[0xFF][0xFF]` suggests a raw struct layout rather than a sparse map.

### 4. AI Table Size by Mode

```cpp
#if DM2_EXTENDED_MODE == 1
    #define CREATURE_AI_TAB_SIZE 64
#else
    #define CREATURE_AI_TAB_SIZE 42
#endif
```
Extended mode adds 22 additional AI entries (likely for DM2's expanded
creature roster). If GDAT is missing or empty, the code falls back to
hardcoded default values — behavior differs between extended and non-extended.

### 5. Save Game I/O Temp File

SKWIN writes a temp file during save/load verification:
```cpp
const char* strSKSaveIOBin = "SKSaveIO.bin";
```
This binary is written to the current working directory (not `C:\A\` as in
original code), and only in `_DEBUG` builds. Release builds skip this step.
If the save fails partial write, the temp file may be left behind.

### 6. Palette Conversion at Blit Time

SKWIN-SDL (`SkwinSDL.cpp`) stores VRAM as 8-bit indexed (`vram[65536]`) but
converts to 32-bit at each `SDL_LockSurface()` blit. The global palette
`glbPaletteRGB[256][3]` is applied during conversion. This means:
- No hardware palette support in SDL path
- Each pixel must be converted individually during scale blit
- Gamma correction via `Gammac()` lookup (`pow(x/63.0, 0.5)`) — square-root gamma

### 7. Allegro Timer vs Game Loop Timing

SKULLWIN uses a 100 Hz Allegro timer (`al_create_timer(0.01)`), but
`al_wait_for_event()` blocks until the next event. This means:
- Timer fires at 10ms intervals
- Game logic must be event-driven (not polling)
- No fixed-timestep guarantees in practice — frame rate = min(60Hz refresh, event rate)
- DM1 uses a more deterministic tick orchestrator (`memory_tick_orchestrator`)

### 8. DM2 V2 Files Mixed with V1

The `src/dm2/` directory contains both V1 and V2 source files:

| V1 files | V2 files |
|----------|----------|
| dm2_v1_game.c | dm2_v2_viewport_renderer.c |
| dm2_v1_dungeon_loader.c | dm2_v2_outdoor_enhanced.c |
| dm2_v1_outdoor_renderer.c | dm2_v2_companion_ui.c |
| dm2_v1_save_load.c | dm2_v2_tech_crafting.c |
| dm2_v1_combat.c | |
| dm2_v1_tech_magic.c | |
| dm2_v1_companion.c | |

CMake correctly separates these: V1 → `firestaff_dm2`, V2 → `firestaff_dm2_v2`.
Never link V2 files into a V1 build, or vice versa.

### 9. M10 Library Dependency

DM1 V1 depends on `firestaff_m10` for shared dungeon decompressor (`dungeon_decompressor_ftl.c`).
DM2 V1 should also depend on `firestaff_m10` for shared utilities, but the current
CMakeLists.txt does NOT link `firestaff_m10` into `firestaff_dm2`:
```cmake
target_link_libraries(firestaff_dm2 PUBLIC firestaff_m10 m)  # MISSING from CMakeLists.txt!
```
This needs to be added for correct build.

### 10. Outdoor State Not Fully Initialized

`dm2_v1_init()` sets `state->outdoor = 0` but does not initialize the outdoor
renderer config. `dm2_v1_outdoor_init()` must be called separately before
entering outdoor areas. Currently no public API function does this
automatically when transitioning from dungeon to outdoor.

### 11. GRAPHICS.DAT Not Currently Loaded in Firestaff V1

`dm2_v1_load_dungeon()` only loads DUNGEON.DAT. The GRAPHICS.DAT file
(8.6 MB) is not loaded in the current V1 code — the hash is listed but
no loading function exists yet. The graphics loading infrastructure
needs to be added (likely as `dm2_v1_graphics_load()`).

### 12. PC-9821 Japanese Variant

DM2 has extensive Japanese PC-9821 support:
- Separate GRAPHICS_PC9821*.dat files (12 MB+)
- Multilanguage release with JP text tables
- DUNGEON_PC9821.dat variant
- These require Shift-JIS text decoding and different font rendering

### 13. Sound Blaster / PC Speaker in DOS Build

SKWINDOS uses `gensound.cpp` for PC speaker beeps and Sound Blaster
digital audio. This is separate from the Windows MIDI system. The DOS
audio is simple square-wave for UI feedback; music requires SB MIDI
or Gravis Ultrasound.

### 14. No OpenGL in Any DM2 Platform

All DM2 platforms use software rendering: Allegro software blit, SDL
software blit, VGA direct write. There is no hardware acceleration,
no OpenGL, no Direct3D. Firestaff's modern SDL2 path is already
more advanced than any original DM2 platform in terms of rendering.
