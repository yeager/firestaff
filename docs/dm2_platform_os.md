# DM2 V1 — Platform/Build Audit: Platform-Specific Code

## Platform-Specific Code

### Original Platforms (skproject)

DM2 shipped on three distinct platforms, each with its own code path:

| Platform     | Toolchain           | Graphics     | Sound         | Resolution  |
|-------------|---------------------|--------------|---------------|-------------|
| SKULLWIN    | Visual C++ (x86)    | Allegro 5    | Allegro audio | 320×200→640×400 |
| SKWIN (MFC) | Visual C++ MFC      | GDI/GUI      | MCI MIDI      | 320×200     |
| SKWIN (SDL) | Visual C++ SDL     | SDL 1.x      | SDL_mixer     | 320×200→1280×800 (scalable) |
| SKWINDOS    | Turbo C / Borland C | DOS VGA      | PC speaker    | 320×200     |

### SKULLWIN (Allegro 5) Platform Code

Key files:
- `c_allegro.cpp` — display init, keyboard/mouse install, event queue
- `c_gfx_main.cpp` — main render loop ( Allegro 5 timer at 100 Hz: `al_create_timer(0.01)` )
- `c_sound.cpp`, `c_midi.cpp` — MIDI playback via Allegro

Display setup:
```cpp
bool windowed = true;
new_swidth = 640; new_sheight = 400; new_colordepth = 32;
al_set_new_display_option(ALLEGRO_COLOR_SIZE, 32, ALLEGRO_REQUIRE);
al_set_new_display_refresh_rate(60);
al5_display = al_create_display(new_swidth, new_sheight);
```
- Render: 320×200 internal (color index 8-bit), scaled 2x to 640×400
- Palette: 256-color indexed (converted to 32-bit at blit time)

### SKWIN-SDL Platform Code

`SkwinSDL.cpp` — CSkWinSDL class:
```cpp
U8 vram[65536];  // 320*200 = 64000 bytes, 8-bit indexed
U8 glbPaletteRGB[256][3];
int sxfact = 1..4;  // scale factor: 1=320x200, 2=640x400, 3=960x600, 4=1280x800
int spfact = 2;  // sprite scale (unused in current code path)
```
Scaling algorithm: floor(y/sxfact), floor(x/sxfact) — nearest-neighbor.

### SKWINDOS (DOS) Platform Code

`SKWINDOS/src/` contains DOS-specific C:
- Direct VGA register access (mode 13h: 320×200×256)
- `int86()` / `int86x()` for BIOS calls
- `gensound.cpp` — PC speaker / Sound Blaster routines

### Platform Differences from DM1

DM1 uses a unified rendering path (shared `bitmap_call` engine) with a single
SDL2 target in Firestaff. DM2 requires separate platform code because:

1. **Multiple graphics backends**: Allegro 5 (SKULLWIN), SDL 1.x (SKWIN-SDL), GDI (SKWIN-MFC), VGA (SKWINDOS)
2. **Multiple audio backends**: Allegro audio vs MCI vs SDL_mixer vs PC speaker
3. **Multiple dungeon sources**: DM2 ships DUNGEON.DAT variants per platform (PC, PC98, beta, demo)
4. **Resolution handling**: DM1 always 320×200. DM2 scales in SDL variant (sxfact)
5. **VRAM layout**: DM2 uses 65536-byte `vram[]` (same as DM1) but with different
   palette update cadence (Allegro vs SDL locksurface)

### Firestaff Platform Abstraction

Firestaff's DM2 V1 code currently only implements the dungeon loader and game
logic skeleton. Platform code (SDL,Allegro) is NOT yet ported — the V1 code
is structured to accept a `data_dir` parameter, keeping platform and logic separate.

Platform code for DM2 in Firestaff is TODO: display, input, audio integration.
