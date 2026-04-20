# M11 — First Playable Layer

**Milestone:** M11 — SDL3 Integration + Filesystem Abstraction + Cross-Platform Build
**Status:** PLAN (not started)
**Created:** 2026-04-20
**Prerequisite:** M10 complete (20/20 phases, 500+ invariants, deterministic tick orchestrator)
**Target:** A human can play Dungeon Master on macOS, Linux, or Windows

---

## Table of Contents

1. [§1 Scope](#1-scope)
2. [§2 Module Decomposition](#2-module-decomposition)
3. [§3 Build System](#3-build-system)
4. [§4 Cross-Platform Checklist](#4-cross-platform-checklist)
5. [§5 Determinism Cross-Validation](#5-determinism-cross-validation)
6. [§6 Implementation Order](#6-implementation-order)
7. [§7 Invariant Philosophy for M11](#7-invariant-philosophy-for-m11)
8. [§8 Risk Register](#8-risk-register)
9. [§9 Acceptance Criteria](#9-acceptance-criteria)
10. [§10 Estimated Effort](#10-estimated-effort)

---

## §1 Scope

### 1.1 What M11 Is

M11 is the **first playable layer**. It takes the pure-C deterministic engine from M10 — which can run 10 000 game ticks headlessly and produce bit-identical world hashes — and adds a presentation shell so a human being can actually *see* the dungeon and *play* the game.

This is about **minimum viable playability**, not polish. The goal is to open a window on three platforms, render something recognisably Dungeon Master, accept keyboard/mouse input, play sounds, and never break M10's determinism invariant in the process.

Concretely, when M11 is done you can:

- Launch `firestaff` on macOS, Linux, or Windows
- See the DM starting screen
- Walk the party through the dungeon using keyboard/mouse
- Hear footsteps, creature sounds, and door mechanisms
- Open/close the inventory, cast spells, engage in combat
- Save and load
- Quit and know the game state is intact

You **cannot** yet:

- Pick between DM1/CSB/DM2 from a startup menu
- Play in Swedish/French/German
- See upscaled or AI-enhanced graphics
- Watch replays in a UI
- Play custom dungeons or mods
- Play multiplayer
- Play on mobile, web, or WASM

### 1.2 In Scope

| Area | Deliverable |
|------|-------------|
| **Rendering** | SDL3 window, software framebuffer, integer scaling (1×–4×), aspect-ratio lock, DM dungeon viewport, UI panels (portraits, inventory, spells, map) |
| **Audio** | SDL3_mixer integration, WAV/VOC playback from DUNGEON.DAT assets, volume mixer (master/SFX/music/UI) |
| **Input** | Keyboard + mouse → `TickInput_Compat` translator, rebindable keys via config |
| **Filesystem** | Portable paths (macOS/Linux/Windows), config file (TOML), save location, asset discovery via MD5 |
| **Build** | CMake replacing shell scripts as canonical build, SDL3 detection with SDL2 fallback |
| **Packaging** | macOS .app + DMG, Linux AppImage + Flatpak + .deb + .rpm, Windows NSIS + portable ZIP |
| **CI/CD** | GitHub Actions on all three platforms, M10 regression + cross-platform hash match |

### 1.3 Out of Scope (Deferred)

| Deferral | Milestone |
|----------|-----------|
| Startup menu (game picker, bug-fix toggles, version presets) | M12 |
| Translation/i18n (sv/fr/de) | M12 |
| Upscaled / AI-HD graphics | M12 |
| Replay playback UI | M12 |
| Chaos Strikes Back / DM2 integration | M13 |
| Custom dungeons | M14+ |
| Networking / co-op | Far future |
| Mobile platforms (iOS/Android) | Far future |
| Web/WASM via Emscripten | Far future |
| Scripting API (Lua/WASM) | Far future |

### 1.4 Architecture Principles (Non-Negotiable)

1. **Layer separation**: M10 is the deterministic data layer. M11 is the presentation layer. M11 depends on M10 via its public API (`memory_tick_orchestrator_pc34_compat.h`). Adding M11 **MUST NOT modify any M10 `.c` or `.h` file**. Period.

2. **Determinism preserved**: The M11 layer is a pure observer of game state (rendering reads world state) and a pure injector of commands (input enqueues `TickInput_Compat` structs). No game-state mutation may occur outside the M10 tick orchestrator.

3. **Cross-platform from day one**: Every feature compiles and runs identically on macOS (Apple Silicon + Intel), Linux (x86_64 + ARM64), Windows (x86_64). Platform-specific code is quarantined in `fs_portable_compat` only.

4. **No `#ifdef __APPLE__` in M11 core**: OS-conditional code lives exclusively in `fs_portable_compat.c`. The rest of M11 calls its portable API.

5. **CMake canonical build**: Shell scripts remain for M10 regression testing (backwards compat). CMake is the single supported build system for M11.

6. **SDL3 preferred, SDL2 fallback**: SDL3 is primary target. SDL2 is supported via compile-time `#if SDL_VERSION_ATLEAST(3,0,0)` gates. No runtime detection — one or the other, chosen at build time.

### 1.5 Interface Contract with M10

M11 talks to M10 exclusively through these interfaces:

| M10 function | M11 usage |
|-------------|-----------|
| `F0882_WORLD_InitFromDungeonDat_Compat` | Load a game from DUNGEON.DAT |
| `F0884_ORCH_AdvanceOneTick_Compat` | Advance one game tick with player input |
| `F0891_ORCH_WorldHash_Compat` | Query world hash for save/verification |
| `F0897_WORLD_Serialize_Compat` / `F0898_WORLD_Deserialize_Compat` | Save/load |
| `F0883_WORLD_Free_Compat` | Cleanup |
| `GameWorld_Compat` struct fields | Read-only access for rendering (party position, champion state, dungeon tile/thing data, monster groups, timeline queue) |

M11 **never** calls M10 internal dispatch functions (`F0887`–`F0890`). M11 **never** directly modifies `GameWorld_Compat` fields — all mutations go through `F0884` via `TickInput_Compat`.

The `TickResult_Compat.emissions[]` array provides event notifications (sound requests, damage dealt, party moved, etc.) that drive M11's audio and visual feedback.

---

## §2 Module Decomposition

M11 introduces ~12 new modules following M10's naming convention. M10 modules use `_pc34_compat` suffix. M11 modules use `_m11` suffix to visually distinguish the layers. All new files live in the same `tmp/firestaff/` directory (flat layout, matching M10 convention).

### 2.1 Module Overview

| # | Module | Files | Purpose | M10 Deps | Est. Size |
|---|--------|-------|---------|----------|-----------|
| 1 | **main_loop_m11** | `main_loop_m11.{h,c}` | Master loop: fixed-step tick + variable-step render + audio pump | Phase 20 (tick orch) | ~600 LOC |
| 2 | **render_sdl_m11** | `render_sdl_m11.{h,c}` | SDL3 window creation, surface/texture management, scaling, present | None (SDL3 only) | ~800 LOC |
| 3 | **render_dungeonview_m11** | `render_dungeonview_m11.{h,c}` | DM's 3D-ish first-person viewport rendering | Phase 1-9 (dungeon data), VGA palette | ~2500 LOC |
| 4 | **render_panels_m11** | `render_panels_m11.{h,c}` | UI panels: portraits, inventory, spell input, map overlay | Phase 10 (champion state), Phase 14 (magic) | ~2000 LOC |
| 5 | **render_text_m11** | `render_text_m11.{h,c}` | Bitmap font renderer (DM's built-in 8×8 font from GRAPHICS.DAT) | GRAPHICS.DAT header | ~400 LOC |
| 6 | **render_sprite_m11** | `render_sprite_m11.{h,c}` | Sprite decode + blit (wall ornaments, monsters, items, effects) | Image backend (IMG3), GRAPHICS.DAT | ~1200 LOC |
| 7 | **audio_sdl_m11** | `audio_sdl_m11.{h,c}` | SDL3_mixer wrapper, DM audio format decoders (WAV/VOC), event-driven playback | Phase 20 (emissions) | ~800 LOC |
| 8 | **input_sdl_m11** | `input_sdl_m11.{h,c}` | SDL event → `TickInput_Compat` translator, mouse hit testing | Phase 20 (TickInput) | ~500 LOC |
| 9 | **input_bindings_m11** | `input_bindings_m11.{h,c}` | Configurable keymap (load/save/rebind) | None | ~400 LOC |
| 10 | **fs_portable_compat** | `fs_portable_compat.{h,c}` | Cross-platform paths, file I/O, user data dir, config dir | None | ~600 LOC |
| 11 | **asset_validator_compat** | `asset_validator_compat.{h,c}` | MD5-based asset discovery and validation | fs_portable_compat | ~500 LOC |
| 12 | **config_m11** | `config_m11.{h,c}` | TOML config file parser + writer | fs_portable_compat, input_bindings | ~700 LOC |

**Total estimated new code: ~11 000 LOC** (excluding tests and probes)

### 2.2 Detailed Module Specifications

---

#### Module 1: `main_loop_m11`

**Purpose:** The master loop that ties M10's deterministic tick engine to SDL3's event-driven presentation. This is the "heartbeat" of the playable game. It implements a semi-fixed timestep: game logic ticks advance at a deterministic rate (matching DM's original ~6 ticks/second), while rendering runs as fast as the display allows (typically vsync'd at 60 Hz).

**Key design decision:** The original DM runs at ~166ms per game tick (≈6 Hz). Between game ticks, M11 renders interpolated frames at the display refresh rate. This means the game looks smooth at 60fps while the engine ticks at 6 Hz — same as the original.

**Public API:**
```c
/* Lifecycle */
int  M11_MainLoop_Init(const char* dungeonPath, uint32_t seed);
void M11_MainLoop_Run(void);
void M11_MainLoop_Shutdown(void);

/* State queries (for debug overlay, etc.) */
int  M11_MainLoop_GetFPS(void);
int  M11_MainLoop_GetGameTick(void);
int  M11_MainLoop_IsRunning(void);
```

**Internal flow (per frame):**
```
1. SDL_PollEvent loop → input_sdl_m11 translator
2. Accumulate time since last game tick
3. If accumulated ≥ TICK_INTERVAL_MS (166ms):
   a. Build TickInput_Compat from queued input
   b. Call F0884_ORCH_AdvanceOneTick_Compat
   c. Process TickResult emissions → audio_sdl_m11
   d. Reset accumulator (minus remainder)
4. render_sdl_m11 → render_dungeonview_m11 + render_panels_m11
5. SDL_RenderPresent
6. Audio pump (SDL_mixer handles async, but check for pending events)
```

**M10 dependencies:** Phase 20 tick orchestrator (F0882, F0884, F0891, F0897/F0898, F0883).

**Files included:** `main_loop_m11.h`, `main_loop_m11.c`

**Estimated size:** ~600 LOC

---

#### Module 2: `render_sdl_m11`

**Purpose:** Owns the SDL3 window, renderer, and the internal 320×200 framebuffer. All other render modules draw into this framebuffer; `render_sdl_m11` handles scaling and presenting it to the actual window.

**Key design decisions:**
- The internal framebuffer is always 320×200 (DM's native resolution), stored as 8-bit indexed colour (4-bit pixel values packed into bytes, matching M10's `screenBitmap` format).
- A second "presentation buffer" is 32-bit RGBA at window resolution, produced by scaling + palette lookup.
- Scaling modes: integer (1×–4×), fit-to-window with aspect lock (16:10 for DM's 320×200), stretch-to-fill.
- Software renderer by default. GPU texture upload for the scaled presentation buffer. No shader-based rendering in M11 (deferred to M12 for upscaling).

**Public API:**
```c
/* Lifecycle */
int  M11_Render_Init(int windowWidth, int windowHeight, int scaleMode);
void M11_Render_Shutdown(void);

/* Framebuffer access */
unsigned char* M11_Render_GetFramebuffer(void);  /* 320×200, 4-bit indexed */
void M11_Render_SetPaletteLevel(int level);       /* 0=brightest, 5=darkest */

/* Direct pixel ops on framebuffer */
void M11_Render_ClearFramebuffer(unsigned char color);
void M11_Render_BlitToFramebuffer(
    const unsigned char* src, int srcW, int srcH,
    int dstX, int dstY, unsigned char transparentColor);

/* Presentation */
void M11_Render_Present(void);  /* Scale framebuffer → window, flip */

/* Window management */
void M11_Render_SetScaleMode(int mode);
void M11_Render_ToggleFullscreen(void);
void M11_Render_HandleResize(int newW, int newH);
SDL_Window* M11_Render_GetWindow(void);
```

**Scaling modes:**
```c
#define M11_SCALE_1X        0
#define M11_SCALE_2X        1
#define M11_SCALE_3X        2
#define M11_SCALE_4X        3
#define M11_SCALE_FIT       4  /* Largest integer scale that fits */
#define M11_SCALE_STRETCH   5  /* Non-integer, fills window */
```

**M10 dependencies:** VGA palette (`vga_palette_pc34_compat.h` — `G9010_auc_VgaPaletteAll_Compat`, `F9010_VGA_GetColorRgb_Compat`). Screen bitmap format (8-bit indexed, same layout as `screen_bitmap_present_pc34_compat.h`).

**SDL3 API surface:**
- `SDL_CreateWindow`, `SDL_CreateRenderer`, `SDL_CreateTexture`
- `SDL_UpdateTexture`, `SDL_RenderTexture`, `SDL_RenderPresent`
- `SDL_SetWindowFullscreen`, `SDL_GetWindowSize`

**SDL2 fallback notes:** `SDL_CreateRenderer` vs `SDL_CreateRenderer` signature differs between SDL2/3. `SDL_RenderTexture` is SDL3; SDL2 uses `SDL_RenderCopy`. The `#if SDL_VERSION_ATLEAST(3,0,0)` gate covers these.

**Files included:** `render_sdl_m11.h`, `render_sdl_m11.c`

**Estimated size:** ~800 LOC

---

#### Module 3: `render_dungeonview_m11`

**Purpose:** Renders the DM first-person 3D-ish viewport — the iconic "looking down a dungeon corridor" view that occupies the left ~2/3 of the screen. This is the most complex rendering module and the visual centrepiece of M11.

**Background:** DM's viewport is not true 3D. It's a pre-computed set of "view slots" at various depths (0–3 squares ahead) and lateral positions (left/centre/right). For each slot, the renderer checks what dungeon element (wall, door, pit, stairs, teleporter, fake wall) is there, then draws the appropriate pre-rendered wall segment, door frame, floor/ceiling, and any ornaments/creatures/items overlaid.

The view is drawn back-to-front (painter's algorithm), depth 3 first, with sprites composited on top. Lighting is handled by selecting one of 6 VGA palette levels (brightest to pitch black) based on the party's active light sources.

**Key data structures:**
```c
/* A single "view slot" in the DM viewport grid */
struct M11_ViewSlot {
    int depth;           /* 0-3 (0 = party square, 3 = farthest) */
    int lateral;         /* -2 to +2 (0 = centre) */
    int worldMapX;       /* Actual dungeon map coordinates */
    int worldMapY;
    int elementType;     /* DUNGEON_ELEMENT_* from M10 */
    int hasCreatures;    /* Monster group present? */
    int hasItems;        /* Floor items present? */
    int ornamentIndex;   /* Wall/floor/door ornament */
    int lightLevel;      /* 0-5 palette index */
};

/* Pre-computed viewport layout (relative offsets per depth/lateral) */
struct M11_ViewportLayout {
    /* Screen coordinates for wall/floor/ceiling segments at each depth */
    int wallLeft[4][5];   /* [depth][lateral] → screen X */
    int wallTop[4][5];    /* [depth][lateral] → screen Y */
    int wallWidth[4][5];
    int wallHeight[4][5];
    int floorY[4];        /* Floor line Y per depth */
    int ceilingY[4];      /* Ceiling line Y per depth */
};
```

**Public API:**
```c
int  M11_DungeonView_Init(void);
void M11_DungeonView_Shutdown(void);

/* Render the viewport for the current party position/facing.
   Draws directly into the M11 framebuffer via render_sdl_m11. */
void M11_DungeonView_Render(
    const struct GameWorld_Compat* world,
    unsigned char* framebuffer);

/* Debug: draw wireframe overlay showing view slot grid */
void M11_DungeonView_RenderDebugGrid(
    unsigned char* framebuffer);
```

**Rendering pipeline:**
1. Compute view slot grid from party position + facing direction (Phase 10 `PartyState_Compat`)
2. For each slot (back-to-front):
   a. Look up dungeon tile at (mapX, mapY) from Phase 1-9 data
   b. Draw wall segments (selecting graphic by wallSet, depth, lateral)
   c. Draw door frames if `DUNGEON_ELEMENT_DOOR`
   d. Draw floor/ceiling based on floorSet
   e. Overlay ornaments (wall ornament index → GRAPHICS.DAT entry)
3. For each slot with creatures (front-to-back for occlusion):
   a. Decode creature sprite from GRAPHICS.DAT
   b. Scale and position based on depth
   c. Blit with transparency
4. For each slot with floor items:
   a. Decode item sprite
   b. Position on floor area
5. Apply lighting: if lightLevel > 0, use darker palette for far squares

**M10 dependencies:**
- Phase 1-9: `DungeonDatState_Compat`, `DungeonThings_Compat`, `DungeonMapTiles_Compat` — tile types, door/ornament data, creature groups, floor items
- Phase 10: `PartyState_Compat` — party position, facing direction, current map index
- Phase 14/19: Active light sources determine palette level per depth
- VGA palette: `G9010_auc_VgaPaletteAll_Compat[6][16][3]` for 6 brightness levels
- GRAPHICS.DAT: `MemoryGraphicsDatHeader_Compat` for sprite metadata, `F0474_MEMORY_LoadGraphic_CPSDF_Compat` for compressed sprite loading, `IMG3_Compat_ExpandFromSource` for decompression
- Image backend: `F0687`–`F0688` nibble/pixel routines, `F0685`/`F0686` line fill/copy

**Reference:** Fontanel's `GRAPH1.C` through `GRAPH4.C` + `PRESENT.C` contain the original rendering logic. DM uses entries 0–558 of GRAPHICS.DAT for wall segments, and 559+ for sprites/UI elements.

**Files included:** `render_dungeonview_m11.h`, `render_dungeonview_m11.c`

**Estimated size:** ~2500 LOC (this is the largest M11 module)

---

#### Module 4: `render_panels_m11`

**Purpose:** Renders all UI panels that surround the dungeon viewport: champion portraits with status bars, the inventory grid, the spell-casting rune panel, and the magic map overlay.

**DM Screen Layout (320×200):**
```
+-------------------+--------+
|                   |Champion|
|                   |Portraits
|   Dungeon         |  (4)   |
|   Viewport        |--------+
|   (224×136)       |Action  |
|                   |Hand    |
+-------------------+--------+
|   Movement  |  Spell/Inv   |
|   Compass   |    Panel     |
+-------------+--------------+
```

The exact pixel coordinates are derived from Fontanel's PRESENT.C / CHAMPION.C screen layout constants.

**Sub-panels:**

1. **Champion portraits** (right side, top): 4 champion slots, each showing:
   - Portrait bitmap (from GRAPHICS.DAT champion entries)
   - Health bar (red), Stamina bar (orange), Mana bar (blue)
   - Name (8 chars, DM bitmap font)
   - Status indicators (poisoned, cursed, dead)

2. **Action hand panel** (right side, middle): Shows selected champion's action hand contents, with left-click = use, right-click = inspect.

3. **Inventory panel** (bottom right, toggled): Grid of inventory slots:
   - Head, Neck, Torso, Legs, Feet (body slots)
   - Pouch ×2, Quiver ×4, Backpack ×3 (storage slots)
   - Each slot shows item sprite or empty outline
   - Drag-and-drop between slots (click-to-pick, click-to-place)

4. **Spell panel** (bottom right, toggled, replaces inventory): 6 rune positions + 4 power levels. Player clicks runes to build spells per Phase 14 `MagicState_Compat`.

5. **Movement compass** (bottom left): The 6-button compass rose (forward, back, strafe-left, strafe-right, turn-left, turn-right). Clickable. Maps to `CMD_MOVE_*` and `CMD_TURN_*`.

6. **Map overlay** (full viewport, toggled): Magic Map / Cartographer mode. Renders a top-down view of explored tiles using Phase 1-9 tile data.

**Public API:**
```c
int  M11_Panels_Init(void);
void M11_Panels_Shutdown(void);

/* Render all visible panels into framebuffer */
void M11_Panels_Render(
    const struct GameWorld_Compat* world,
    unsigned char* framebuffer);

/* Toggle which panel is visible in the bottom-right area */
void M11_Panels_ShowInventory(int championIndex);
void M11_Panels_ShowSpellPanel(int championIndex);
void M11_Panels_ToggleMap(void);

/* Hit testing for mouse input */
int  M11_Panels_HitTest(int mouseX, int mouseY,
    struct M11_PanelHitResult* outResult);
```

**M10 dependencies:**
- Phase 10: `ChampionState_Compat` (hp, stamina, mana, attributes, inventory slots, name)
- Phase 14: `MagicState_Compat` (rune buffer, castable spells)
- Phase 1-9: Tile data for map overlay, item data for inventory display
- GRAPHICS.DAT: Portrait entries, item sprites, rune icons, panel backgrounds

**Files included:** `render_panels_m11.h`, `render_panels_m11.c`

**Estimated size:** ~2000 LOC

---

#### Module 5: `render_text_m11`

**Purpose:** Bitmap font renderer using DM's built-in 8×8 pixel font extracted from GRAPHICS.DAT. No dependency on SDL_ttf — we use the authentic DM font for all in-game text.

The DM font is a 1-bit 8×8 glyph set stored in GRAPHICS.DAT (entries vary by version; typically in the "UI element" range). Characters are uppercase only in the original (A-Z, 0-9, and a handful of symbols). M11 extends with lowercase by mirroring uppercase glyphs (matching original DM behaviour where text was always uppercase).

**Public API:**
```c
int  M11_Text_Init(void);  /* Load font glyphs from GRAPHICS.DAT */
void M11_Text_Shutdown(void);

/* Draw text string into framebuffer at (x,y) using specified color */
void M11_Text_Draw(
    unsigned char* framebuffer,
    int x, int y,
    const char* text,
    unsigned char color);

/* Measure text width in pixels (8px per character) */
int M11_Text_MeasureWidth(const char* text);

/* Draw text centred horizontally within a region */
void M11_Text_DrawCentered(
    unsigned char* framebuffer,
    int regionX, int regionWidth, int y,
    const char* text,
    unsigned char color);
```

**M10 dependencies:** GRAPHICS.DAT header + font glyph entries. Text string decoding from Phase 7 (`F0507_DUNGEON_DecodeTextAtOffset_Compat`).

**Files included:** `render_text_m11.h`, `render_text_m11.c`

**Estimated size:** ~400 LOC

---

#### Module 6: `render_sprite_m11`

**Purpose:** High-level sprite management — loads, caches, and blits decoded GRAPHICS.DAT entries. Acts as the bridge between M10's compressed image pipeline (`image_backend_pc34_compat.h`, `memory_graphics_dat_header_pc34_compat.h`) and M11's framebuffer.

M10 already has the full IMG3 decompression pipeline (`IMG3_Compat_ExpandFromSource`, nibble decoder, line fill/copy). This module wraps that pipeline with a decoded-sprite cache so sprites aren't re-decompressed every frame.

**Key data structures:**
```c
#define M11_SPRITE_CACHE_SIZE 256

struct M11_Sprite {
    unsigned char* pixels;      /* Decoded 4-bit indexed pixels */
    int width;
    int height;
    int graphicIndex;           /* GRAPHICS.DAT entry index */
    int valid;
};

struct M11_SpriteCache {
    struct M11_Sprite entries[M11_SPRITE_CACHE_SIZE];
    int usedCount;
    int lruOrder[M11_SPRITE_CACHE_SIZE];  /* LRU eviction */
};
```

**Public API:**
```c
int  M11_Sprite_Init(const char* graphicsDatPath);
void M11_Sprite_Shutdown(void);

/* Get a decoded sprite (loads + caches if not present) */
const struct M11_Sprite* M11_Sprite_Get(int graphicIndex);

/* Blit sprite to framebuffer with optional transparency */
void M11_Sprite_Blit(
    const struct M11_Sprite* sprite,
    unsigned char* framebuffer,
    int dstX, int dstY,
    unsigned char transparentColor);

/* Blit sprite with scaling (for depth-dependent sizing in viewport) */
void M11_Sprite_BlitScaled(
    const struct M11_Sprite* sprite,
    unsigned char* framebuffer,
    int dstX, int dstY, int dstW, int dstH,
    unsigned char transparentColor);

/* Flush entire cache (e.g., on palette change or map transition) */
void M11_Sprite_FlushCache(void);
```

**M10 dependencies:**
- `memory_graphics_dat_pc34_compat.h`: `F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat`, `F0474_MEMORY_LoadGraphic_CPSDF_Compat`
- `memory_graphics_dat_header_pc34_compat.h`: `F0479_MEMORY_LoadGraphicsDatHeader_Compat` (sprite dimensions, compressed sizes)
- `image_backend_pc34_compat.h`: `IMG3_Compat_ExpandFromSource` (decompression)
- `memory_frontend_pc34_compat.h`: `F0488_MEMORY_ExpandGraphicToBitmap_Compat`

**Files included:** `render_sprite_m11.h`, `render_sprite_m11.c`

**Estimated size:** ~1200 LOC

---

#### Module 7: `audio_sdl_m11`

**Purpose:** Audio playback driven by `TickResult_Compat.emissions[]`. When the tick orchestrator emits `EMIT_SOUND_REQUEST` events, this module translates them to SDL3_mixer channel playback.

**DM Audio Formats:**

DM PC 3.4 uses a small set of sound effects stored either:
- Embedded in DUNGEON.DAT (unlikely for PC 3.4 — needs verification)
- As separate .WAV files (Creative Labs Sound Blaster format)
- As .VOC files (Creative Voice File, a Creative Labs format)

**Note:** The exact audio format and storage location for PC 3.4 sounds requires disassembly verification. Fontanel's ReDMCSB documentation suggests Atari ST used chip sounds; the PC version added digitised SFX. We must verify what the PC 3.4 binary references before implementation. See Risk R4.

**Sound categories (mapped from emission payloads):**
- Party movement (footsteps on different floor types)
- Door opening/closing/bashing
- Creature sounds (type-specific roars, attacks, death)
- Combat hit/miss/parry
- Spell casting (per spell school)
- Item pickup/drop/throw
- Champion damage/death/revival
- UI clicks (button press, inventory slot)
- Ambient (dungeon hum — may not exist in original PC 3.4)

**Public API:**
```c
int  M11_Audio_Init(void);
void M11_Audio_Shutdown(void);

/* Process emissions from a tick result */
void M11_Audio_ProcessEmissions(
    const struct TickResult_Compat* result);

/* Direct sound playback (for UI sounds not driven by ticks) */
void M11_Audio_PlayUISound(int soundId);

/* Volume control (0.0–1.0) */
void M11_Audio_SetMasterVolume(float volume);
void M11_Audio_SetSFXVolume(float volume);
void M11_Audio_SetMusicVolume(float volume);
void M11_Audio_SetUIVolume(float volume);

/* Mute/unmute */
void M11_Audio_ToggleMute(void);
```

**M10 dependencies:** Phase 20 `TickResult_Compat` (emissions with `EMIT_SOUND_REQUEST`). Sound ID mapping TBD (needs PC 3.4 disassembly of sound table).

**SDL3 API surface:**
- `SDL_mixer`: `Mix_OpenAudio`, `Mix_LoadWAV`, `Mix_PlayChannel`, `Mix_Volume`, `Mix_FreeChunk`
- SDL3_mixer is a separate library (`SDL3_mixer`), distinct from SDL3 core

**Files included:** `audio_sdl_m11.h`, `audio_sdl_m11.c`

**Estimated size:** ~800 LOC

---

#### Module 8: `input_sdl_m11`

**Purpose:** Translates SDL3 keyboard/mouse events into `TickInput_Compat` command structs that the M10 tick orchestrator consumes. This is the "write path" — the only way M11 injects intent into the game state.

**Design:** Input events are collected during the SDL event loop (called every frame at display refresh rate) and buffered. When the main loop is ready to advance a game tick, the buffer is drained into a single `TickInput_Compat`. If multiple commands are queued between ticks, only the most recent takes priority (DM is one-command-per-tick).

**Mouse hit testing:** The mouse position is translated to the 320×200 framebuffer coordinate space (accounting for scaling), then passed to `render_panels_m11`'s hit-test API to determine what UI element was clicked. Clicks on the dungeon viewport itself are ignored in M11 (no 3D-space mouse interaction in DM1).

**Public API:**
```c
int  M11_Input_Init(void);
void M11_Input_Shutdown(void);

/* Process all pending SDL events. Called once per frame.
   Returns 0 if quit requested. */
int  M11_Input_ProcessEvents(void);

/* Drain buffered input into a TickInput for the next game tick.
   Clears the buffer after drain. */
void M11_Input_DrainToTickInput(struct TickInput_Compat* outInput);

/* Query immediate mouse state (for UI hover effects) */
void M11_Input_GetMousePosition(int* outX, int* outY);
int  M11_Input_IsMouseButtonDown(int button);

/* Special keys (not game commands) */
int  M11_Input_WasKeyPressed(int sdlKeycode);  /* One-shot per frame */
```

**Command mapping (default):**

| SDL Key | Game Command | Notes |
|---------|-------------|-------|
| W / Up | `CMD_MOVE_NORTH` (relative to facing) | Forward |
| S / Down | `CMD_MOVE_SOUTH` | Backward |
| A | `CMD_MOVE_WEST` | Strafe left |
| D | `CMD_MOVE_EAST` | Strafe right |
| Q / Left | `CMD_TURN_LEFT` | |
| E / Right | `CMD_TURN_RIGHT` | |
| Space | `CMD_ATTACK` | Selected champion |
| Enter | `CMD_USE_ITEM` | Action hand |
| R | `CMD_REST_TOGGLE` | |
| F5 | Quick save | Not a game command — handled in main_loop |
| F9 | Quick load | Not a game command — handled in main_loop |
| F11 | Toggle fullscreen | Not a game command — handled in render_sdl |
| Esc | Pause/menu | Not a game command — handled in main_loop |

Mouse clicks on compass buttons, inventory slots, champion portraits, and spell runes generate the appropriate command via panel hit testing.

**M10 dependencies:** Phase 20 `TickInput_Compat` struct (command codes `CMD_*`).

**Files included:** `input_sdl_m11.h`, `input_sdl_m11.c`

**Estimated size:** ~500 LOC

---

#### Module 9: `input_bindings_m11`

**Purpose:** Configurable key/mouse bindings. Loads from config file, allows runtime rebinding, saves changes. Separates the "what key was pressed" from "what game action does it mean" so Module 8 doesn't hardcode key mappings.

**Public API:**
```c
int  M11_Bindings_Init(void);
void M11_Bindings_Shutdown(void);

/* Load bindings from config (or use defaults if no config) */
int  M11_Bindings_LoadFromConfig(const struct M11_Config* config);
int  M11_Bindings_SaveToConfig(struct M11_Config* config);

/* Translate SDL keycode/scancode to game command */
uint8_t M11_Bindings_TranslateKey(SDL_Keycode key);
uint8_t M11_Bindings_TranslateScancode(SDL_Scancode scancode);

/* Rebind a command to a new key */
int  M11_Bindings_Rebind(uint8_t command, SDL_Keycode newKey);

/* Reset to defaults */
void M11_Bindings_ResetDefaults(void);

/* Query: what key is bound to this command? */
SDL_Keycode M11_Bindings_GetKeyForCommand(uint8_t command);
const char* M11_Bindings_GetKeyNameForCommand(uint8_t command);
```

**M10 dependencies:** None directly. Uses `CMD_*` constants from Phase 20 header.

**Files included:** `input_bindings_m11.h`, `input_bindings_m11.c`

**Estimated size:** ~400 LOC

---

#### Module 10: `fs_portable_compat`

**Purpose:** THE platform abstraction layer. All `#ifdef _WIN32`, `#ifdef __APPLE__`, `#ifdef __linux__` code in the entire M11 codebase lives here and only here. Every other module calls these portable functions.

**Path conventions:**
```
macOS:
  User data: ~/Library/Application Support/Firestaff/
  Config:    ~/Library/Application Support/Firestaff/config.toml
  Saves:     ~/Library/Application Support/Firestaff/saves/
  Log:       ~/Library/Application Support/Firestaff/firestaff.log

Linux (XDG):
  User data: $XDG_DATA_HOME/firestaff/  (default: ~/.local/share/firestaff/)
  Config:    $XDG_CONFIG_HOME/firestaff/config.toml  (default: ~/.config/firestaff/)
  Saves:     $XDG_DATA_HOME/firestaff/saves/
  Log:       $XDG_DATA_HOME/firestaff/firestaff.log

Windows:
  User data: %APPDATA%\Firestaff\
  Config:    %APPDATA%\Firestaff\config.toml
  Saves:     %APPDATA%\Firestaff\saves\
  Log:       %APPDATA%\Firestaff\firestaff.log
```

**Public API:**
```c
/* Path queries */
int  FSP_GetUserDataDir(char* outBuf, int bufSize);
int  FSP_GetConfigDir(char* outBuf, int bufSize);
int  FSP_GetSaveDir(char* outBuf, int bufSize);
int  FSP_GetLogPath(char* outBuf, int bufSize);

/* Path manipulation */
int  FSP_JoinPath(const char* base, const char* relative,
                  char* outBuf, int bufSize);
int  FSP_GetBasename(const char* path, char* outBuf, int bufSize);
int  FSP_NormalizePath(const char* path, char* outBuf, int bufSize);
int  FSP_PathExists(const char* path);
int  FSP_IsDirectory(const char* path);
int  FSP_IsFile(const char* path);

/* Directory operations */
int  FSP_CreateDirectoryRecursive(const char* path);
int  FSP_ListDirectory(const char* path,
                       char*** outEntries, int* outCount);
void FSP_FreeDirectoryList(char** entries, int count);

/* File I/O (portable wrappers) */
int  FSP_ReadFileToBuffer(const char* path,
                          unsigned char** outBuf, size_t* outSize);
int  FSP_WriteBufferToFile(const char* path,
                           const unsigned char* buf, size_t size);

/* Encoding */
int  FSP_Utf8ToNativePath(const char* utf8Path,
                          char* outNative, int nativeSize);

/* Platform info */
const char* FSP_GetPlatformName(void);  /* "macOS", "Linux", "Windows" */
```

**Design notes:**
- All paths in Firestaff are UTF-8 internally. On Windows, `FSP_Utf8ToNativePath` converts to wide chars for Win32 API calls.
- Path separator is always `/` internally. Conversion to `\` happens only inside `fs_portable_compat.c` on Windows.
- `FSP_CreateDirectoryRecursive` creates all intermediate directories (like `mkdir -p`).
- No C runtime `fopen` on Windows for paths with non-ASCII characters — use `_wfopen` internally.

**M10 dependencies:** None. This is a standalone platform abstraction.

**Files included:** `fs_portable_compat.h`, `fs_portable_compat.c`

**Estimated size:** ~600 LOC

---

#### Module 11: `asset_validator_compat`

**Purpose:** Discovers and validates original game data files (DUNGEON.DAT, GRAPHICS.DAT) by MD5 hash. The user points Firestaff at a directory containing their legally owned game files; this module scans it, identifies which game version(s) are present, and reports readiness.

**Known MD5 hashes (to be populated from community databases):**

```c
/* DM PC 3.4 EN (I34E) */
#define MD5_DUNGEON_DAT_DM_PC34_EN   "..." /* TBD: compute from known-good file */
#define MD5_GRAPHICS_DAT_DM_PC34_EN  "..." /* TBD */

/* CSB PC (if different DAT files) */
#define MD5_DUNGEON_DAT_CSB_PC       "..." /* TBD */

/* Additional variants: Atari ST, Amiga, etc. */
/* These are detected but not playable in M11. */
```

**Public API:**
```c
struct M11_AssetInfo {
    char path[512];
    char md5[33];       /* Hex string */
    int  gameId;        /* DUNGEON_ID_DM, DUNGEON_ID_CSB_*, etc. */
    int  recognized;    /* 1 if MD5 matches a known version */
    const char* label;  /* "DM PC 3.4 EN", "CSB PC", etc. */
};

int  M11_AssetValidator_Init(void);
void M11_AssetValidator_Shutdown(void);

/* Scan a directory for game data files */
int  M11_AssetValidator_ScanDirectory(
    const char* dirPath,
    struct M11_AssetInfo* outAssets,
    int maxAssets,
    int* outFoundCount);

/* Validate a specific file by path */
int  M11_AssetValidator_ValidateFile(
    const char* filePath,
    struct M11_AssetInfo* outInfo);

/* Compute MD5 of a file (utility) */
int  M11_AssetValidator_ComputeMD5(
    const char* filePath,
    char* outMd5Hex);
```

**M10 dependencies:** None directly. Uses `fs_portable_compat` for file I/O.

**Dependencies:** MD5 implementation (a small embedded md5.c, ~200 LOC, public domain).

**Files included:** `asset_validator_compat.h`, `asset_validator_compat.c`, `md5_compat.h`, `md5_compat.c`

**Estimated size:** ~500 LOC (excluding md5_compat)

---

#### Module 12: `config_m11`

**Purpose:** TOML-format configuration file parser and writer. Stores all user preferences: window size, scale mode, volume levels, key bindings, game data directory, last save slot, etc.

**Why TOML:** TOML is human-readable, human-editable, well-defined, and simpler than YAML (no gotchas with boolean interpretation, no indentation sensitivity). A minimal TOML parser for C is ~500 LOC and has no dependencies.

**Config file structure (`config.toml`):**
```toml
[general]
game_data_dir = "/path/to/game/files"
last_game = "dm_pc34_en"

[display]
window_width = 960
window_height = 600
scale_mode = "fit"         # "1x", "2x", "3x", "4x", "fit", "stretch"
fullscreen = false
vsync = true

[audio]
master_volume = 0.8
sfx_volume = 1.0
music_volume = 0.7
ui_volume = 0.5
muted = false

[input]
# Key bindings (SDL keycode names)
move_forward = "w"
move_backward = "s"
strafe_left = "a"
strafe_right = "d"
turn_left = "q"
turn_right = "e"
attack = "space"
use_item = "return"
rest = "r"

[debug]
show_fps = false
show_tick = false
wireframe_viewport = false
```

**Public API:**
```c
struct M11_Config {
    /* General */
    char gameDataDir[512];
    char lastGame[64];

    /* Display */
    int windowWidth;
    int windowHeight;
    int scaleMode;
    int fullscreen;
    int vsync;

    /* Audio */
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    float uiVolume;
    int muted;

    /* Input (SDL keycodes) */
    int keyBindings[32];  /* Indexed by CMD_* constants */

    /* Debug */
    int showFps;
    int showTick;
    int wireframeViewport;
};

int  M11_Config_Init(struct M11_Config* config);  /* Set defaults */
int  M11_Config_Load(const char* path, struct M11_Config* config);
int  M11_Config_Save(const char* path, const struct M11_Config* config);
```

**M10 dependencies:** None. Uses `fs_portable_compat` for path resolution.

**TOML parser:** Embedded minimal parser (handles strings, integers, floats, booleans, sections). No arrays, no inline tables, no date-times — we don't need them. ~500 LOC. Alternatively, use a public-domain single-header library like `toml.c`.

**Files included:** `config_m11.h`, `config_m11.c`, `toml_parser_m11.h`, `toml_parser_m11.c`

**Estimated size:** ~700 LOC (including TOML parser)

---

### 2.3 Module Dependency Graph

```
                    main_loop_m11
                   /     |      \
                  /      |       \
        render_sdl_m11   |    input_sdl_m11
           /    |        |        |
          /     |        |    input_bindings_m11
         /      |        |        |
render_dungeonview_m11   |    config_m11
        |                |        |
render_panels_m11        |    fs_portable_compat
        |                |        |
render_text_m11    audio_sdl_m11  asset_validator_compat
        |
render_sprite_m11
        |
    [M10 modules — read-only]
```

**Rule:** Arrows point down (dependency direction). No cycles. No M11 module depends on another M11 module in the same horizontal layer. `main_loop_m11` is the top-level orchestrator. `fs_portable_compat` is the bottom (platform abstraction).

---

## §3 Build System

### 3.1 CMakeLists.txt Layout

CMake replaces the shell-script build as the canonical way to build Firestaff. The existing shell scripts (`run_firestaff_m10_verify.sh`, `run_firestaff_headless_driver.sh`) remain for M10 regression testing but are no longer the primary build path.

**Directory structure (after M11):**

```
tmp/firestaff/
├── CMakeLists.txt              # Top-level CMake
├── cmake/
│   ├── FindSDL3.cmake          # SDL3 find module (if not in CMake's built-in)
│   ├── FindSDL3_mixer.cmake    # SDL3_mixer find module
│   ├── CompilerWarnings.cmake  # Shared warning flags
│   └── PackagingConfig.cmake   # CPack configuration
├── src/
│   └── m11/
│       ├── CMakeLists.txt      # M11 module build rules
│       ├── main_loop_m11.c
│       ├── main_loop_m11.h
│       ├── render_sdl_m11.c
│       ├── ... (all M11 modules)
│       └── main.c              # Entry point for `firestaff` binary
├── src/
│   └── m10/                    # Symlink or reference to M10 sources
│       └── (all *_pc34_compat.{h,c} files)
├── tests/
│   ├── CMakeLists.txt
│   ├── m10_probes/             # All 20 M10 probe binaries
│   └── m11_probes/             # New M11 probe binaries
├── assets/
│   ├── firestaff.desktop       # Linux .desktop file
│   ├── firestaff.png           # App icon (256×256)
│   ├── firestaff.icns          # macOS icon
│   ├── firestaff.ico           # Windows icon
│   └── firestaff.1             # Man page
├── packaging/
│   ├── macos/
│   │   ├── Info.plist
│   │   └── entitlements.plist
│   ├── linux/
│   │   ├── AppDir/             # AppImage layout
│   │   ├── flatpak/
│   │   │   └── se.firestaff.Firestaff.yml
│   │   ├── deb/
│   │   └── rpm/
│   └── windows/
│       └── firestaff.nsi       # NSIS installer script
├── run_firestaff_m10_verify.sh # Legacy (kept for CI)
├── run_firestaff_headless_driver.sh
└── ... (existing M10 files)
```

**Note:** In the initial implementation, all files will remain in the flat `tmp/firestaff/` directory to match M10 convention. The `src/m10/`, `src/m11/`, and `tests/` subdirectory structure is introduced by Phase H (CMake) and files are moved then. Until that phase, the CMakeLists.txt uses `file(GLOB ...)` on the flat directory.

### 3.2 Top-Level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(Firestaff VERSION 0.11.0 LANGUAGES C)

# ──── Options ────
option(FIRESTAFF_USE_SDL2 "Build with SDL2 instead of SDL3" OFF)
option(FIRESTAFF_BUILD_HEADLESS "Build headless regression binary" ON)
option(FIRESTAFF_BUILD_TESTS "Build test/probe binaries" ON)
option(FIRESTAFF_BUILD_M10_VERIFY "Build all 20 M10 probe binaries" ON)

# ──── Compiler settings ────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
include(cmake/CompilerWarnings.cmake)

# ──── SDL detection ────
if(NOT FIRESTAFF_USE_SDL2)
    find_package(SDL3 QUIET)
    if(SDL3_FOUND)
        find_package(SDL3_mixer QUIET)
        set(FIRESTAFF_SDL_VERSION 3)
    else()
        message(WARNING "SDL3 not found, falling back to SDL2")
        set(FIRESTAFF_USE_SDL2 ON)
    endif()
endif()

if(FIRESTAFF_USE_SDL2)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    set(FIRESTAFF_SDL_VERSION 2)
    add_compile_definitions(FIRESTAFF_SDL2_FALLBACK=1)
endif()

# ──── M10 library (deterministic engine core) ────
file(GLOB M10_SOURCES "${CMAKE_SOURCE_DIR}/*_pc34_compat.c"
                      "${CMAKE_SOURCE_DIR}/*_pc34.c")
file(GLOB M10_HEADERS "${CMAKE_SOURCE_DIR}/*_pc34_compat.h"
                      "${CMAKE_SOURCE_DIR}/*_pc34.h")
add_library(firestaff_m10 STATIC ${M10_SOURCES})
target_include_directories(firestaff_m10 PUBLIC ${CMAKE_SOURCE_DIR})
# M10 has no dependencies — pure C, no libraries

# ──── M11 library (presentation layer) ────
set(M11_SOURCES
    main_loop_m11.c
    render_sdl_m11.c
    render_dungeonview_m11.c
    render_panels_m11.c
    render_text_m11.c
    render_sprite_m11.c
    audio_sdl_m11.c
    input_sdl_m11.c
    input_bindings_m11.c
    fs_portable_compat.c
    asset_validator_compat.c
    config_m11.c
    toml_parser_m11.c
    md5_compat.c
)
add_library(firestaff_m11 STATIC ${M11_SOURCES})
target_link_libraries(firestaff_m11 PUBLIC firestaff_m10)
if(FIRESTAFF_SDL_VERSION EQUAL 3)
    target_link_libraries(firestaff_m11 PUBLIC SDL3::SDL3 SDL3_mixer::SDL3_mixer)
else()
    target_link_libraries(firestaff_m11 PUBLIC SDL2::SDL2 SDL2_mixer::SDL2_mixer)
endif()

# ──── Main executable ────
add_executable(firestaff main_m11.c)
target_link_libraries(firestaff PRIVATE firestaff_m11)

# ──── Headless regression binary ────
if(FIRESTAFF_BUILD_HEADLESS)
    add_executable(firestaff_headless firestaff_headless_driver.c)
    target_link_libraries(firestaff_headless PRIVATE firestaff_m10)
    # Note: headless does NOT link SDL — pure M10 only
endif()

# ──── M10 probe binaries (for CI) ────
if(FIRESTAFF_BUILD_M10_VERIFY)
    file(GLOB M10_PROBES "${CMAKE_SOURCE_DIR}/firestaff_m10_*_probe.c")
    foreach(probe_src ${M10_PROBES})
        get_filename_component(probe_name ${probe_src} NAME_WE)
        add_executable(${probe_name} ${probe_src})
        target_link_libraries(${probe_name} PRIVATE firestaff_m10)
    endforeach()
endif()

# ──── M11 probe binaries ────
if(FIRESTAFF_BUILD_TESTS)
    file(GLOB M11_PROBES "${CMAKE_SOURCE_DIR}/firestaff_m11_*_probe.c")
    foreach(probe_src ${M11_PROBES})
        get_filename_component(probe_name ${probe_src} NAME_WE)
        add_executable(${probe_name} ${probe_src})
        target_link_libraries(${probe_name} PRIVATE firestaff_m11)
    endforeach()
endif()

# ──── Install ────
install(TARGETS firestaff RUNTIME DESTINATION bin)
install(TARGETS firestaff_headless RUNTIME DESTINATION bin OPTIONAL)
install(FILES assets/firestaff.1 DESTINATION share/man/man1 OPTIONAL)
if(UNIX AND NOT APPLE)
    install(FILES assets/firestaff.desktop
            DESTINATION share/applications)
    install(FILES assets/firestaff.png
            DESTINATION share/icons/hicolor/256x256/apps)
endif()

# ──── CPack ────
include(cmake/PackagingConfig.cmake)
```

### 3.3 Package Discovery

**SDL3 detection strategy:**

1. Try CMake's built-in `find_package(SDL3)` (SDL3 ships a `SDL3Config.cmake` since its release).
2. If not found and `FIRESTAFF_USE_SDL2` is OFF, issue a warning and fall back to SDL2.
3. SDL3 installation sources:
   - **macOS:** `brew install sdl3 sdl3_mixer`
   - **Linux:** System package (`libsdl3-dev`) or build from source. SDL3 may not be in Ubuntu 22.04 repos — vcpkg or FetchContent fallback.
   - **Windows:** vcpkg (`vcpkg install sdl3 sdl3-mixer`) or manual download to `CMAKE_PREFIX_PATH`.

**SDL2 fallback:**

The SDL2 fallback is a compile-time option for systems where SDL3 is unavailable. All SDL API calls are wrapped in a thin compatibility layer:

```c
/* In render_sdl_m11.c (and similar): */
#if FIRESTAFF_SDL2_FALLBACK
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_mixer.h>
  /* SDL2 compat macros */
  #define SDL_RenderTexture SDL_RenderCopy
  /* ... */
#else
  #include <SDL3/SDL.h>
  #include <SDL3/SDL_mixer.h>
#endif
```

### 3.4 Build Targets

| Target | Type | Links | Purpose |
|--------|------|-------|---------|
| `firestaff` | Executable | M10 + M11 + SDL3 | Main game binary |
| `firestaff_headless` | Executable | M10 only | Headless regression/replay tool |
| `firestaff_m10` | Static lib | None | M10 deterministic engine |
| `firestaff_m11` | Static lib | M10 + SDL3 | M11 presentation layer |
| `firestaff_m10_*_probe` | Executables (20) | M10 | Phase 1-20 invariant probes |
| `firestaff_m11_*_probe` | Executables (12-16) | M11 | Phase A-P invariant probes |

### 3.5 Install Layout

**macOS (.app bundle):**
```
Firestaff.app/
├── Contents/
│   ├── Info.plist
│   ├── MacOS/
│   │   └── firestaff        # Main binary
│   ├── Resources/
│   │   └── firestaff.icns   # App icon
│   └── Frameworks/
│       ├── SDL3.framework/
│       └── SDL3_mixer.framework/
```

**Linux (FHS):**
```
/usr/bin/firestaff
/usr/bin/firestaff_headless
/usr/share/man/man1/firestaff.1
/usr/share/applications/firestaff.desktop
/usr/share/icons/hicolor/256x256/apps/firestaff.png
```

**Windows:**
```
C:\Program Files\Firestaff\
├── firestaff.exe
├── firestaff_headless.exe
├── SDL3.dll
├── SDL3_mixer.dll
└── README.txt
```

### 3.6 Compiler Warnings (`cmake/CompilerWarnings.cmake`)

```cmake
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter      # M10 has unused params in stubs
        -Werror=implicit-function-declaration
        -Werror=return-type
        -Wuninitialized
        -Wshadow
        -Wformat=2
        -Wconversion
        -Wno-sign-conversion       # M10 uses mixed signed/unsigned
    )
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
    endif()
elseif(MSVC)
    add_compile_options(
        /W4
        /WX         # Warnings as errors
        /wd4100     # Unreferenced formal parameter
        /wd4244     # Possible loss of data (int → short)
        /wd4267     # size_t → int
    )
endif()
```

---

## §4 Cross-Platform Checklist

### 4.1 macOS

| Item | Value |
|------|-------|
| **Minimum OS** | macOS 13 (Ventura) — SDL3 requires macOS 10.15+, but Ventura is reasonable for 2026 |
| **Architectures** | Apple Silicon (arm64) + Intel (x86_64) — Universal Binary via `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` |
| **Build toolchain** | Xcode 15+ command-line tools (Clang 15+) |
| **SDL3 source** | `brew install sdl3 sdl3_mixer` |
| **Packaging** | `.app` bundle (CMake + `MACOSX_BUNDLE`), DMG via `create-dmg` or `hdiutil` |
| **Code signing** | Ad-hoc signing for dev builds (`codesign -s -`). Proper signing requires Apple Developer account ($99/yr). Notarisation requires `xcrun notarytool submit`. |
| **Known quirks** | |

**macOS-specific quirks:**
1. **Retina scaling:** macOS reports logical points, not physical pixels. A 960×600 window on a Retina display is actually 1920×1200 pixels. SDL3's `SDL_GetWindowSizeInPixels` returns the real pixel count. Our integer scaling must account for this — scale factor = physical pixels / 320, not logical points / 320.
2. **App Translocation:** Downloaded unsigned apps are quarantined and run from a random `/private/var/` path. `FSP_GetUserDataDir` must not be relative to the binary path.
3. **Metal vs OpenGL:** SDL3 defaults to Metal on macOS. Our software renderer uploads a texture each frame — Metal or OpenGL is fine, SDL abstracts this.
4. **Audio entitlement:** No special entitlements needed for SDL_mixer audio playback.
5. **Universal Binary caveat:** SDL3 frameworks from Homebrew are typically arm64-only. For true Universal Binary, need to build SDL3 from source with both architectures or use the official SDL3 macOS framework download.

### 4.2 Linux

| Item | Value |
|------|-------|
| **Minimum OS** | Ubuntu 22.04 LTS (glibc 2.35), Fedora 38+, Debian 12+ |
| **Architectures** | x86_64 (primary), ARM64/aarch64 (secondary — Raspberry Pi 4+) |
| **Build toolchain** | GCC 12+ or Clang 14+ |
| **SDL3 source** | System package (`libsdl3-dev`) if available; otherwise `vcpkg` or build from source. On Ubuntu 22.04, SDL3 is definitely not in repos — use PPA, snap, or FetchContent. |
| **Packaging** | AppImage (`linuxdeploy`), Flatpak, .deb (via CPack or `dpkg-deb`), .rpm (via CPack or `rpmbuild`) |
| **Known quirks** | |

**Linux-specific quirks:**
1. **Wayland vs X11:** SDL3 supports both. Test on both. Wayland window positioning is non-deterministic (compositor decides). X11 has legacy fullscreen quirks. SDL3's `SDL_HINT_VIDEO_DRIVER` can force one or the other.
2. **PulseAudio vs PipeWire vs ALSA:** SDL3_mixer abstracts this, but test all three backends. PipeWire is default on Fedora 38+ and Ubuntu 22.10+. Older Ubuntu 22.04 uses PulseAudio.
3. **AppImage glibc:** AppImage bundles may need older glibc for broad compatibility. Build on Ubuntu 22.04 for maximum compat.
4. **Flatpak sandboxing:** Flatpak restricts filesystem access. The `--filesystem=host:ro` permission or a file chooser portal is needed for the user to point at their game data directory.
5. **High DPI:** Linux DPI handling varies wildly (GNOME scales, KDE has per-display scaling, etc.). SDL3's `SDL_GetDisplayContentScale` helps but isn't universally reliable. Default to integer scaling and let the user override.
6. **ARM64 considerations:** Raspberry Pi 4 has limited GPU. Software rendering is preferred. SDL3 on ARM64 Linux works but may need Mesa/DRM backend instead of X11.

### 4.3 Windows

| Item | Value |
|------|-------|
| **Minimum OS** | Windows 10 (build 1809+) — SDL3 requires Windows 7+ but we target 10 for modern path APIs |
| **Architectures** | x86_64 only (32-bit Windows is dead in 2026) |
| **Build toolchain** | MSVC 2022 (cl.exe 19.3+), or Clang-CL, or MinGW-w64. MSVC is primary. |
| **SDL3 source** | vcpkg (`vcpkg install sdl3 sdl3-mixer`), or manual download from libsdl.org, or submodule |
| **Packaging** | NSIS installer, portable ZIP (just the exe + DLLs), optional WiX MSI |
| **Known quirks** | |

**Windows-specific quirks:**
1. **Path separators:** All M11 code uses `/` internally. `fs_portable_compat.c` converts to `\` only for Win32 API calls. This is transparent to all other modules.
2. **Unicode paths:** Windows has `_wfopen` for Unicode file names. `fs_portable_compat.c` converts UTF-8 → UTF-16 via `MultiByteToWideChar` before all file operations. M10's `fopen` calls work because DUNGEON.DAT paths are typically ASCII, but `fs_portable_compat` must handle non-ASCII user names (e.g., `C:\Users\Ångström\AppData\...`).
3. **DPI awareness:** Set `<dpiAware>true/pm</dpiAware>` in the application manifest (embedded in the exe via `.rc` file). Without this, Windows scales the window bitmap and it looks blurry at 125%/150%.
4. **Console window:** By default, MSVC builds `firestaff` as a Windows (GUI) subsystem app. The headless driver is a console app. Use `/SUBSYSTEM:WINDOWS` for the main binary.
5. **DLL deployment:** SDL3.dll and SDL3_mixer.dll must be in the same directory as the exe, or in PATH. The NSIS installer handles this. The portable ZIP includes them.
6. **MSVC struct packing:** M10 uses explicit byte sizes and no compiler packing pragmas. Verify with a static assert that `sizeof(TickInput_Compat) == 16` on MSVC. If MSVC pads differently, add `#pragma pack(push, 1)` guards in the _compat headers (this is the one exception to "M10 files are read-only" — but only if padding is wrong, verified in Phase J CI).
7. **Sleep precision:** Windows' default timer resolution is 15.6ms. For smooth 60fps, call `timeBeginPeriod(1)` at startup (or rely on SDL3's internal handling of this).

### 4.4 Platform Test Matrix

| Feature | macOS arm64 | macOS x86_64 | Linux x86_64 | Linux arm64 | Windows x86_64 |
|---------|------------|-------------|-------------|------------|---------------|
| Window creation | ✓ | ✓ | ✓ | ✓ | ✓ |
| Integer scaling | ✓ | ✓ | ✓ | ✓ | ✓ |
| Fullscreen | ✓ | ✓ | ✓ | ✓ | ✓ |
| Audio playback | ✓ | ✓ | ✓ | ✓ | ✓ |
| Keyboard input | ✓ | ✓ | ✓ | ✓ | ✓ |
| Mouse input | ✓ | ✓ | ✓ | ✓ | ✓ |
| User data dir | ~/Library/... | ~/Library/... | XDG | XDG | %APPDATA% |
| Config R/W | ✓ | ✓ | ✓ | ✓ | ✓ |
| Save/Load | ✓ | ✓ | ✓ | ✓ | ✓ |
| World hash match | Must be identical across all 5 columns |

---

## §5 Determinism Cross-Validation

### 5.1 The Critical Invariant

> Running `firestaff_headless --dungeon DUNGEON.DAT --seed 42 --ticks 100` on macOS, Linux, and Windows **MUST** produce the **exact same** final world hash.

This is not optional. This is not "nice to have". This is the foundation that makes replay verification, cross-platform save compatibility, and bug-reproduction possible. If this invariant breaks, we stop and fix it before anything else.

### 5.2 Why It Should Work (and What Could Break It)

**Why it should work:**

M10's design is explicitly hostile to non-determinism:
- **No floating point:** All arithmetic is integer. Borland LCG RNG uses uint32_t multiplication + addition. No FPU involved.
- **Explicit endianness:** All serialisation is LSB-first (matching PC's Little-Endian), written byte-by-byte, not via `*(uint32_t*)ptr`. Endian-correct on all platforms.
- **Explicit struct sizes:** Every serialised struct has a defined byte count (`TICK_INPUT_SERIALIZED_SIZE = 16`, etc.). The serialiser writes/reads individual fields, not raw struct memcpy.
- **No undefined behaviour:** No uninitialised reads (verified by Clang's `-Wuninitialized`), no signed integer overflow (checked), no pointer aliasing violations.
- **No OS calls:** M10 uses only `fopen`/`fread`/`fwrite`/`malloc`/`free`/`memset`/`memcpy` — all fully deterministic.

**What could break it:**

| Risk | Likelihood | Mitigation |
|------|-----------|------------|
| **Struct padding** | Low | M10 serialises field-by-field, not `memcpy(&struct, buf)`. But `sizeof(GameWorld_Compat)` may differ across compilers — hashing the struct via `F0891` must hash fields, not raw bytes. **Verify with static asserts.** |
| **Signed integer representation** | Negligible | All modern platforms use two's complement. C11 doesn't mandate it, but C23 does. All our targets (Clang, GCC, MSVC) use two's complement. |
| **`malloc` address-dependent behaviour** | None | M10 never hashes pointers or uses pointer ordering for game logic. The `dungeonFingerprint` is CRC32 of file contents, not pointer-derived. |
| **`qsort` instability** | Possible | If M10 uses `qsort` internally, different C libraries may order equal elements differently. **Verify M10 doesn't use qsort** (it shouldn't — timeline queue uses explicit priority insertion). |
| **Compiler optimisations** | Unlikely | Aggressive optimisation could reorder operations, but with no floats and no UB, the result is identical. Verify with `-O0` vs `-O2` hash comparison. |
| **MSVC struct packing** | Possible | MSVC default packing may differ from GCC/Clang for structs with mixed-size fields. The hash function must not depend on in-memory layout. **Add static asserts for critical struct sizes.** |

### 5.3 Test Harness Design

**`firestaff_determinism_crosscheck.sh` (runs in CI):**

```bash
#!/bin/bash
set -euo pipefail

DUNGEON_DAT="$1"
SEEDS=(1 42 1337 65535 4294967295)
TICK_COUNTS=(10 100 1000 10000)
HASH_FILE="determinism_hashes_$(uname -s)_$(uname -m).txt"

echo "# Firestaff determinism cross-check" > "$HASH_FILE"
echo "# Platform: $(uname -s) $(uname -m)" >> "$HASH_FILE"
echo "# Compiler: $(cc --version | head -1)" >> "$HASH_FILE"
echo "# Date: $(date -u +%Y-%m-%dT%H:%M:%SZ)" >> "$HASH_FILE"

for seed in "${SEEDS[@]}"; do
    for ticks in "${TICK_COUNTS[@]}"; do
        HASH=$(./firestaff_headless \
            --dungeon "$DUNGEON_DAT" \
            --seed "$seed" \
            --ticks "$ticks" \
            | grep "^FinalHash:" | awk '{print $2}')
        echo "seed=${seed} ticks=${ticks} hash=${HASH}" >> "$HASH_FILE"
    done
done
```

This script runs on each CI platform. A final comparison step diffs the three hash files and fails if they differ.

### 5.4 CI Workflow for Determinism

```yaml
# .github/workflows/determinism.yml
name: Cross-Platform Determinism Check
on: [push, pull_request]

jobs:
  build-and-hash:
    strategy:
      matrix:
        os: [macos-14, ubuntu-22.04, windows-2022]
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4

      - name: Build headless
        run: |
          cmake -B build -DFIRESTAFF_BUILD_HEADLESS=ON \
                         -DFIRESTAFF_BUILD_TESTS=OFF
          cmake --build build --target firestaff_headless

      - name: Run determinism check
        run: |
          bash scripts/determinism_crosscheck.sh \
            test-data/DUNGEON.DAT

      - name: Upload hashes
        uses: actions/upload-artifact@v4
        with:
          name: determinism-${{ matrix.os }}
          path: determinism_hashes_*.txt

  compare:
    needs: build-and-hash
    runs-on: ubuntu-latest
    steps:
      - uses: actions/download-artifact@v4

      - name: Compare hashes across platforms
        run: |
          # Extract just the hash lines, sort, and compare
          for f in determinism-*/determinism_hashes_*.txt; do
            grep "^seed=" "$f" | sort
          done | uniq -u > diffs.txt

          if [ -s diffs.txt ]; then
            echo "DETERMINISM FAILURE — hashes differ:"
            cat diffs.txt
            exit 1
          fi
          echo "All platform hashes match. Determinism verified."
```

### 5.5 Known Determinism Risks (Detailed)

1. **SDL internal float usage (R2):** Even though M10 uses no floats, M11 calls SDL for rendering. But since M11 only reads world state for rendering and only writes `TickInput_Compat` for input, SDL's internal float usage (in audio resampling, texture scaling, etc.) cannot affect game state. The only path from M11 to M10 is the `TickInput_Compat` struct, which contains only integer fields. **Risk: None for determinism. Confirmed by architecture.**

2. **Uninitialised memory on different platforms:** M10 uses `memset(&world, 0, sizeof(world))` and `calloc`. On all three platforms, this zeros all bytes including padding. The hash function (`F0891`) hashes specific fields, not raw bytes. **Risk: Negligible. Verify F0891 doesn't hash padding.**

3. **Timer-dependent behaviour:** M10's tick orchestrator is step-count-based, not wall-clock-based. `F0884_ORCH_AdvanceOneTick_Compat` advances exactly one tick regardless of how long it took. **Risk: None.**

4. **File I/O differences:** `fread` on Windows opens files in text mode by default (translating `\r\n`). M10's `F0500_DUNGEON_LoadDatHeader_Compat` must open in binary mode (`"rb"`). **Verify all `fopen` calls use `"rb"` / `"wb"`.** If any use `"r"` / `"w"`, this is a determinism bug on Windows.

---

## §6 Implementation Order

M11 is implemented in 16 phases (A through P). Each phase produces:
1. A probe binary (`firestaff_m11_<name>_probe.c`) with invariant checks
2. A verification script entry
3. A documented invariant list

Phases are sequential — each depends on the previous. However, some phases can be parallelised (noted below).

### Phase A — SDL3 Window + Basic Framebuffer

**Goal:** Open an SDL3 window and display a solid-color or gradient pattern at 320×200.

**Modules touched:** `render_sdl_m11`

**Deliverables:**
- SDL3 window at 640×400 (2× scale, hardcoded)
- Internal 320×200 8-bit framebuffer
- Palette lookup from VGA palette (brightest level)
- Present: scale framebuffer → window texture → display
- ESC to quit

**Probe:** `firestaff_m11_window_probe.c`

**Invariants (10):**
1. `INV_A01`: Window created successfully (SDL_CreateWindow returns non-NULL)
2. `INV_A02`: Renderer created successfully
3. `INV_A03`: Framebuffer is exactly 320×200 bytes
4. `INV_A04`: Palette lookup maps color 0 → RGB(0,0,0) (black)
5. `INV_A05`: Palette lookup maps color 15 → RGB(255,255,255) (white at brightest)
6. `INV_A06`: All 6 palette levels load without error
7. `INV_A07`: Clear framebuffer sets all 64000 bytes to specified color
8. `INV_A08`: Present doesn't crash with empty framebuffer
9. `INV_A09`: Present doesn't crash with fully populated framebuffer
10. `INV_A10`: Window resize callback fires and updates internal dimensions

**Effort:** 2 days

---

### Phase B — Scaling + Aspect Ratio

**Goal:** Support all scaling modes (1×–4×, fit, stretch) and aspect-ratio-locked display.

**Modules touched:** `render_sdl_m11` (extend)

**Deliverables:**
- Integer scaling at 1×, 2×, 3×, 4×
- Fit-to-window with letterboxing/pillarboxing (16:10 aspect ratio for 320×200)
- Stretch-to-fill (no aspect lock)
- Fullscreen toggle (F11)
- Keyboard shortcut to cycle scale modes (F10)
- Retina/HiDPI handling (macOS, Windows)

**Probe:** `firestaff_m11_scaling_probe.c`

**Invariants (8):**
1. `INV_B01`: 1× scale: window content area is 320×200 logical pixels
2. `INV_B02`: 2× scale: window content area is 640×400
3. `INV_B03`: 4× scale: window content area is 1280×800
4. `INV_B04`: Fit mode: framebuffer aspect ratio 16:10 is preserved (within 1px tolerance)
5. `INV_B05`: Stretch mode: framebuffer fills entire window (no letterboxing)
6. `INV_B06`: Fullscreen toggle doesn't crash or lose framebuffer contents
7. `INV_B07`: Retina: physical pixel count is 2× logical on Retina displays
8. `INV_B08`: Mouse coordinates correctly map from window space to framebuffer space at all scale modes

**Effort:** 1.5 days

---

### Phase C — Filesystem Abstraction

**Goal:** Implement `fs_portable_compat` so all subsequent phases can use portable paths.

**Modules touched:** `fs_portable_compat`

**Deliverables:**
- Platform-correct user data dir, config dir, save dir
- Directory creation (recursive)
- Path join, basename, normalise
- File read/write wrappers
- UTF-8 path handling (Windows)

**Probe:** `firestaff_m11_filesystem_probe.c`

**Invariants (10):**
1. `INV_C01`: `FSP_GetUserDataDir` returns a non-empty path
2. `INV_C02`: Path contains platform-correct separator on output (even though internal is `/`)
3. `INV_C03`: `FSP_CreateDirectoryRecursive` creates nested directories
4. `INV_C04`: `FSP_JoinPath("a", "b")` produces `"a/b"` internally
5. `INV_C05`: `FSP_PathExists` returns true for created directory
6. `INV_C06`: `FSP_ReadFileToBuffer` → `FSP_WriteBufferToFile` round-trips data
7. `INV_C07`: `FSP_NormalizePath` collapses `..` and `.` segments
8. `INV_C08`: `FSP_GetBasename("/foo/bar.txt")` returns `"bar.txt"`
9. `INV_C09`: `FSP_ListDirectory` returns all entries in a populated directory
10. `INV_C10`: UTF-8 path with non-ASCII characters works on all platforms

**Effort:** 2 days

---

### Phase D — Config + TOML Parser

**Goal:** Load and save configuration. Establish the `config.toml` format.

**Modules touched:** `config_m11`, `toml_parser_m11`

**Deliverables:**
- Minimal TOML parser (sections, key-value pairs: strings, ints, floats, bools)
- Config struct with sane defaults
- Load from file, save to file
- Config file auto-created with defaults on first run

**Probe:** `firestaff_m11_config_probe.c`

**Invariants (8):**
1. `INV_D01`: Default config has valid values for all fields
2. `INV_D02`: Config save → load round-trips all fields
3. `INV_D03`: TOML parser handles quoted strings with spaces
4. `INV_D04`: TOML parser handles integer values (positive and negative)
5. `INV_D05`: TOML parser handles float values (0.0 to 1.0)
6. `INV_D06`: TOML parser handles boolean values (true/false)
7. `INV_D07`: TOML parser ignores comments (lines starting with `#`)
8. `INV_D08`: Missing config file → defaults used without error

**Effort:** 1.5 days

---

### Phase E — Asset Validation

**Goal:** MD5-based game data file discovery and validation.

**Modules touched:** `asset_validator_compat`, `md5_compat`

**Deliverables:**
- MD5 computation for files
- Directory scan for DUNGEON.DAT / GRAPHICS.DAT
- Known-hash lookup table (populated from test files)
- Report: which game versions are present

**Probe:** `firestaff_m11_asset_validator_probe.c`

**Invariants (7):**
1. `INV_E01`: MD5 of empty file matches known empty-file hash
2. `INV_E02`: MD5 of known test string matches RFC 1321 test vector
3. `INV_E03`: Directory scan finds DUNGEON.DAT in test directory
4. `INV_E04`: Recognised file returns `recognized == 1` and correct game ID
5. `INV_E05`: Unrecognised file (wrong MD5) returns `recognized == 0`
6. `INV_E06`: Non-existent directory returns 0 found files without crash
7. `INV_E07`: Large file (>1MB) computes MD5 without memory issues

**Effort:** 1 day

---

### Phase F — Sprite Loading + Cache

**Goal:** Load and cache decoded GRAPHICS.DAT sprites for rendering.

**Modules touched:** `render_sprite_m11`

**Deliverables:**
- Open GRAPHICS.DAT, read header (entry count, sizes)
- Decode individual sprites via IMG3 pipeline
- LRU sprite cache (256 entries)
- Blit sprite to framebuffer (with transparency)
- Scaled blit (for depth-dependent sizing)

**Probe:** `firestaff_m11_sprite_probe.c`

**Invariants (10):**
1. `INV_F01`: GRAPHICS.DAT opens and header loads successfully
2. `INV_F02`: Header reports correct graphic count (>500 for DM PC 3.4)
3. `INV_F03`: Entry 0 decodes without crash
4. `INV_F04`: Decoded sprite has expected width/height from header
5. `INV_F05`: All pixels in decoded sprite are in range 0-15 (4-bit palette)
6. `INV_F06`: Sprite cache stores and retrieves entries by graphic index
7. `INV_F07`: Cache evicts LRU entry when full (after 256 unique loads)
8. `INV_F08`: Blit to framebuffer writes correct pixels at correct positions
9. `INV_F09`: Blit with transparency skips transparent-color pixels
10. `INV_F10`: Scaled blit produces output at correct dimensions

**Effort:** 2 days

---

### Phase G — Text Rendering

**Goal:** DM bitmap font rendering.

**Modules touched:** `render_text_m11`

**Deliverables:**
- Extract font glyphs from GRAPHICS.DAT
- Render text strings to framebuffer
- Measure text width
- Centred text rendering

**Probe:** `firestaff_m11_text_probe.c`

**Invariants (6):**
1. `INV_G01`: Font glyphs load from GRAPHICS.DAT
2. `INV_G02`: Character 'A' renders an 8×8 glyph (non-zero pixels)
3. `INV_G03`: Text width of "HELLO" is 40 pixels (5 × 8)
4. `INV_G04`: DrawCentered places text at correct x offset
5. `INV_G05`: Unknown characters render as blank (no crash)
6. `INV_G06`: Empty string renders nothing (no crash)

**Effort:** 1 day

---

### Phase H — Input Handler + Key Bindings

**Goal:** SDL event processing and game command translation.

**Modules touched:** `input_sdl_m11`, `input_bindings_m11`

**Deliverables:**
- SDL event loop processing
- Keyboard → command translation via binding table
- Mouse position → framebuffer coordinate mapping
- Default key bindings (WASD + QE + arrows)
- Binding save/load integration with config

**Probe:** `firestaff_m11_input_probe.c`

**Invariants (8):**
1. `INV_H01`: Default bindings map W → CMD_MOVE_NORTH
2. `INV_H02`: Default bindings map Q → CMD_TURN_LEFT
3. `INV_H03`: Rebind works: after rebinding, new key produces correct command
4. `INV_H04`: Old key produces CMD_NONE after rebinding
5. `INV_H05`: DrainToTickInput produces a valid TickInput_Compat struct
6. `INV_H06`: TickInput from DrainToTickInput has correct tick number
7. `INV_H07`: Mouse coordinate mapping correct at 2× scale (framebuffer coords = window / 2)
8. `INV_H08`: Reset-to-defaults restores original binding table

**Effort:** 1.5 days

---

### Phase I — Main Loop + Integration

**Goal:** Wire together tick orchestrator, rendering, input, and audio into the game loop.

**Modules touched:** `main_loop_m11`

**Deliverables:**
- Fixed-step game tick at ~6 Hz
- Variable-step rendering at display refresh rate
- Event loop → input → tick → render → present cycle
- Load DUNGEON.DAT on startup
- FPS counter (debug toggle)
- Quick save (F5) / quick load (F9)

**Probe:** `firestaff_m11_mainloop_probe.c`

**Invariants (8):**
1. `INV_I01`: Game world initialises from DUNGEON.DAT without error
2. `INV_I02`: 100 game ticks advance without crash
3. `INV_I03`: World hash after 100 CMD_NONE ticks matches headless driver hash
4. `INV_I04`: World hash after 100 CMD_NONE ticks matches across platforms (determinism)
5. `INV_I05`: Render loop runs at >30 FPS on reference hardware
6. `INV_I06`: Save → load → 100 ticks produces same hash as continuous 200 ticks
7. `INV_I07`: Quit (ESC + confirm) exits cleanly without leak
8. `INV_I08`: FPS counter toggles on/off without affecting game state

**Effort:** 2 days

---

### Phase J — Dungeon Viewport Renderer

**Goal:** Render the 3D-ish first-person dungeon view.

**Modules touched:** `render_dungeonview_m11`

**Deliverables:**
- View slot computation from party position + facing
- Wall segment rendering (by wall set, depth, lateral)
- Floor/ceiling rendering
- Door frame rendering
- Ornament overlay
- Creature sprite rendering (at depth-correct scale)
- Floor item rendering
- Lighting (palette level per depth based on active lights)

**Probe:** `firestaff_m11_dungeonview_probe.c`

**Invariants (12):**
1. `INV_J01`: View slot grid computed correctly for north-facing party at (0,0)
2. `INV_J02`: View slot grid rotates correctly for each facing direction
3. `INV_J03`: Wall at depth 0 renders at correct screen coordinates
4. `INV_J04`: Door renders with correct ornament at depth 1
5. `INV_J05`: Creature sprite renders at depth-appropriate scale
6. `INV_J06`: Floor items render in correct cell position
7. `INV_J07`: Lighting: palette level 0 (brightest) at depth 0 with torch
8. `INV_J08`: Lighting: palette level 5 (darkest) at depth 3 with no light
9. `INV_J09`: Empty corridor renders floor, ceiling, and no walls (correct geometry)
10. `INV_J10`: Golden image: starting position viewport matches reference PNG (pixel-identical)
11. `INV_J11`: Pit renders as floor gap (no wall segments at pit tile)
12. `INV_J12`: Stairs render with correct up/down visual based on direction

**Effort:** 5 days (this is the hardest phase)

---

### Phase K — UI Panels

**Goal:** Render champion portraits, inventory, spell panel, movement compass, map overlay.

**Modules touched:** `render_panels_m11`

**Deliverables:**
- Champion portrait display (4 slots)
- Health/stamina/mana bars
- Inventory grid with item sprites
- Spell rune panel
- Movement compass (clickable)
- Map overlay (toggle)
- Hit testing for mouse interaction

**Probe:** `firestaff_m11_panels_probe.c`

**Invariants (10):**
1. `INV_K01`: 4 champion slots render at correct screen positions
2. `INV_K02`: Health bar width proportional to HP/maxHP
3. `INV_K03`: Empty inventory slot renders outline (no crash)
4. `INV_K04`: Inventory slot with weapon renders correct item sprite
5. `INV_K05`: Spell panel shows 6 rune positions
6. `INV_K06`: Compass hit-test: click at compass centre → CMD_MOVE_NORTH
7. `INV_K07`: Compass hit-test: click at compass right → CMD_TURN_RIGHT
8. `INV_K08`: Map overlay renders explored tiles
9. `INV_K09`: Map overlay toggle on/off works without crash
10. `INV_K10`: Panel layout matches DM's original 320×200 screen divisions (within 2px)

**Effort:** 4 days

---

### Phase L — Audio Integration

**Goal:** Sound effects driven by tick emissions.

**Modules touched:** `audio_sdl_m11`

**Deliverables:**
- SDL3_mixer initialisation
- Sound effect loading (WAV/VOC from game data)
- Emission → sound mapping
- Volume mixer (master, SFX, music, UI)
- Mute toggle

**Probe:** `firestaff_m11_audio_probe.c`

**Invariants (7):**
1. `INV_L01`: SDL3_mixer initialises without error
2. `INV_L02`: WAV file loads and reports correct duration
3. `INV_L03`: Sound plays on correct channel without crash
4. `INV_L04`: Volume setting 0.0 produces silence (channel volume = 0)
5. `INV_L05`: Volume setting 1.0 produces max channel volume
6. `INV_L06`: Mute → unmute restores previous volume levels
7. `INV_L07`: Playing 10 simultaneous sounds doesn't crash (channel limit handling)

**Effort:** 2 days

---

### Phase M — CMake Build System

**Goal:** Full CMake build replacing shell scripts for M11.

**Modules touched:** `CMakeLists.txt`, `cmake/*.cmake`

**Deliverables:**
- Top-level CMakeLists.txt
- SDL3 detection with SDL2 fallback
- All build targets (firestaff, firestaff_headless, probes)
- Install rules
- CPack configuration

**Probe:** Build system is its own test — successful compilation of all targets on all platforms IS the invariant.

**Invariants (8):**
1. `INV_M01`: `cmake -B build` configures without error on macOS
2. `INV_M02`: `cmake -B build` configures without error on Linux
3. `INV_M03`: `cmake -B build` configures without error on Windows
4. `INV_M04`: `cmake --build build` compiles all targets with zero warnings on macOS (Clang)
5. `INV_M05`: `cmake --build build` compiles all targets with zero warnings on Linux (GCC)
6. `INV_M06`: `cmake --build build` compiles all targets with zero warnings on Windows (MSVC)
7. `INV_M07`: `cmake --install build` produces correct install layout
8. `INV_M08`: SDL2 fallback build succeeds when SDL3 is absent

**Effort:** 2 days

---

### Phase N — CI/CD Pipeline

**Goal:** GitHub Actions workflow for build, test, and release.

**Modules touched:** `.github/workflows/*.yml`

**Deliverables:**
- Build workflow (all 3 platforms × 2 configs: Release + Debug)
- M10 verify workflow (all 20 probes on all 3 platforms)
- M11 verify workflow (all M11 probes on all 3 platforms)
- Determinism cross-check workflow
- Release workflow (auto-tag + build artifacts)

**Probe:** CI green on all platforms IS the invariant.

**Invariants (8):**
1. `INV_N01`: macOS build succeeds
2. `INV_N02`: Linux build succeeds
3. `INV_N03`: Windows build succeeds
4. `INV_N04`: All 20 M10 probes pass on all platforms
5. `INV_N05`: All M11 probes pass on all platforms
6. `INV_N06`: Cross-platform determinism check passes
7. `INV_N07`: Release build produces artifacts for all 3 platforms
8. `INV_N08`: Release artifacts are downloadable and have correct file sizes

**Effort:** 2 days

---

### Phase O — Packaging

**Goal:** Distribution-ready packages for all platforms.

**Modules touched:** `packaging/*`

**Deliverables:**
- macOS: .app bundle + DMG (unsigned or ad-hoc signed)
- Linux: AppImage + .deb + .rpm
- Windows: NSIS installer + portable ZIP
- Flatpak manifest (Linux)

**Probe:** Package builds successfully on each platform and launches.

**Invariants (6):**
1. `INV_O01`: macOS .app bundle launches and shows window
2. `INV_O02`: DMG mounts and contains .app
3. `INV_O03`: AppImage launches on Ubuntu 22.04
4. `INV_O04`: .deb installs on Ubuntu 22.04 without dependency errors
5. `INV_O05`: Windows NSIS installer runs and installs to Program Files
6. `INV_O06`: Windows portable ZIP extracts and exe launches

**Effort:** 3 days

---

### Phase P — Integration Testing + Polish

**Goal:** End-to-end testing, final bug fixes, documentation.

**Modules touched:** All

**Deliverables:**
- Full playthrough test (walk from start, pick up items, fight, cast spells, save, load)
- Performance profiling (target: 60fps on 2020-era hardware)
- Memory leak check (Valgrind on Linux, Instruments on macOS)
- README update with M11 build instructions
- CHANGELOG.md for v0.11.0
- Screenshot / demo recording for GitHub

**Invariants (6):**
1. `INV_P01`: Full playthrough (100 ticks with movement + combat) completes without crash
2. `INV_P02`: Memory usage stays under 50MB during normal gameplay
3. `INV_P03`: No memory leaks (Valgrind clean or <100 bytes reachable)
4. `INV_P04`: Frame time p99 < 16ms (60fps) on reference hardware
5. `INV_P05`: All M10 probes still pass (regression check)
6. `INV_P06`: README build instructions work on a clean checkout

**Effort:** 3 days

---

### Phase Summary

| Phase | Name | Modules | Invariants | Effort |
|-------|------|---------|------------|--------|
| A | SDL3 Window | render_sdl_m11 | 10 | 2 days |
| B | Scaling | render_sdl_m11 | 8 | 1.5 days |
| C | Filesystem | fs_portable_compat | 10 | 2 days |
| D | Config + TOML | config_m11, toml_parser | 8 | 1.5 days |
| E | Asset Validation | asset_validator, md5 | 7 | 1 day |
| F | Sprite Loading | render_sprite_m11 | 10 | 2 days |
| G | Text Rendering | render_text_m11 | 6 | 1 day |
| H | Input Handler | input_sdl, input_bindings | 8 | 1.5 days |
| I | Main Loop | main_loop_m11 | 8 | 2 days |
| J | Dungeon Viewport | render_dungeonview_m11 | 12 | 5 days |
| K | UI Panels | render_panels_m11 | 10 | 4 days |
| L | Audio | audio_sdl_m11 | 7 | 2 days |
| M | CMake Build | CMakeLists.txt | 8 | 2 days |
| N | CI/CD | .github/workflows | 8 | 2 days |
| O | Packaging | packaging/* | 6 | 3 days |
| P | Integration Test | All | 6 | 3 days |
| **Total** | | **12 modules** | **132** | **34.5 days** |

### Parallelisation Opportunities

Some phases are independent and can run in parallel if multiple developers are working:

- **C + D + E** (filesystem, config, asset validation) are independent of A + B (rendering)
- **F + G** (sprites, text) depend on A but not on each other
- **H** (input) depends on A (for SDL init) but not on F/G
- **M** (CMake) can start as soon as A + C compile, and evolve through remaining phases
- **N** (CI) can start in skeleton form at Phase M and be extended as probes are added

The critical path is: **A → F → J → K → I → P** (window → sprites → dungeon render → panels → main loop → integration). This path is ~18 days.

---

## §7 Invariant Philosophy for M11

### 7.1 M10 vs M11: Different Beasts

M10's invariants are beautiful: pure-function assertions. Given this input, the output is exactly this. Bit-identical. Cross-platform. No ambiguity.

M11 adds a visual, auditory, temporal layer. Invariants must adapt:

| Domain | M10 approach | M11 approach |
|--------|-------------|-------------|
| Data correctness | Hash comparison | Hash comparison (unchanged for game state) |
| Visual output | N/A | Golden image comparison (PNG reference frames) |
| Audio output | N/A | Cross-correlation with reference WAV |
| Timing | N/A | Frame-time percentile thresholds |
| Input | N/A | Round-trip tests (event → command → tick → state) |
| Paths | N/A | Round-trip through platform-specific dirs |
| Determinism | Hash across runs | Hash across platforms |

### 7.2 Golden Image Testing

For deterministic scenarios (specific world state, specific party position), the rendered framebuffer is **pixel-identical**. This is because:
- The framebuffer is 320×200 indexed colour, not interpolated
- Sprite decoding is deterministic (IMG3 is a fixed algorithm)
- Palette lookup is a table, not computation
- No anti-aliasing, no blending, no alpha

**Golden image workflow:**
1. Set up a test world (known DUNGEON.DAT, seed 42, tick 0, party at start position facing north)
2. Render one frame to the 320×200 framebuffer
3. Export framebuffer to PNG (via a test utility)
4. On first run: save as `golden/m11_start_north.png`
5. On subsequent runs: compare pixel-by-pixel. Any difference = FAIL.

**Reference images are checked into the repo.** They're small (~10KB each for 320×200 indexed colour PNG).

The golden image test runs as part of CI. It catches:
- Rendering regressions (sprite offset changes, palette errors)
- Platform divergence (if any rendering path differs)

### 7.3 Frame Timing Invariants

Not all environments can guarantee 60fps (CI runners are often slow). Timing invariants are therefore **advisory on CI** and **enforced locally**:

```
INV: Frame render time p99 < 16ms on reference hardware
INV: Game tick processing time < 5ms per tick (measured on headless)
INV: SDL event processing time < 1ms per frame
```

In CI, we relax these to "did not crash" and "completed in under 60 seconds for 1000 frames".

### 7.4 Audio Invariants

Audio is the hardest to test deterministically. Our approach:

1. **Emission-based testing:** Given a sequence of ticks, verify the `EMIT_SOUND_REQUEST` emissions match expected (this is M10-level, fully deterministic).
2. **Playback smoke test:** Verify SDL_mixer returns success when asked to play a sound.
3. **Cross-correlation deferred:** Comparing actual audio output against a reference WAV requires capturing the SDL audio output buffer. This is complex and deferred to M12. M11 tests that the right sound IDs are requested at the right ticks.

### 7.5 Input Round-Trip Invariants

The input invariant chain:

```
SDL_KEYDOWN(W) → M11_Bindings_TranslateKey → CMD_MOVE_NORTH
  → TickInput_Compat{.command=0x01}
  → F0884_ORCH_AdvanceOneTick_Compat
  → world.party.mapX changes (if move succeeds)
  → world hash changes
```

Test: inject a synthetic `SDL_KEYDOWN` event (via `SDL_PushEvent`), process it through the full chain, and verify the world hash changes as expected. This is end-to-end input testing.

### 7.6 Path Round-Trip Invariants

```
FSP_GetUserDataDir → "~/Library/Application Support/Firestaff/"
FSP_JoinPath(dir, "config.toml") → "~/Library/.../Firestaff/config.toml"
FSP_WriteBufferToFile(path, data) → file written
FSP_ReadFileToBuffer(path) → data matches original
```

Tested on all platforms in CI.

### 7.7 Invariant Count

| Phase | Invariants |
|-------|-----------|
| A | 10 |
| B | 8 |
| C | 10 |
| D | 8 |
| E | 7 |
| F | 10 |
| G | 6 |
| H | 8 |
| I | 8 |
| J | 12 |
| K | 10 |
| L | 7 |
| M | 8 |
| N | 8 |
| O | 6 |
| P | 6 |
| **Total** | **132** |

Target was ≥100. We exceed it at **132 planned invariants**.

---

## §8 Risk Register

### R1: SDL3 vs SDL2 API Drift (Severity: Medium, Likelihood: Medium)

**Description:** SDL3 was released January 2025. By April 2026 it should be stable, but the API has changed significantly from SDL2. The SDL2 fallback path requires maintaining two code paths for renderer creation, event handling, and audio init. API drift may cause subtle bugs that only manifest on one SDL version.

**Impact:** Double the testing surface for rendering and audio. Bugs in the SDL2 path may go unnoticed if developers primarily test SDL3.

**Mitigation:**
- CI tests both SDL2 and SDL3 builds (separate matrix entries)
- Compat macros are concentrated in `render_sdl_m11.c` and `audio_sdl_m11.c` only — not spread throughout
- SDL2 fallback is a best-effort tier: if something breaks only on SDL2, we document it and suggest SDL3
- Consult SDL migration guide: https://wiki.libsdl.org/SDL3/README/migration

**Contingency:** If SDL3 adoption is universal by the time M11 ships, drop SDL2 fallback entirely (reduces maintenance burden).

### R2: Cross-Platform Float Determinism via SDL (Severity: Low, Likelihood: Low)

**Description:** SDL internally uses floats for audio resampling, texture coordinate interpolation, and scaling calculations. If any float result leaks into game state, cross-platform determinism breaks.

**Impact:** World hash mismatch across platforms.

**Mitigation:**
- Architecture already prevents this: M11 → M10 communication is exclusively via `TickInput_Compat` (integer fields only)
- M11 never modifies `GameWorld_Compat` directly
- The world hash (`F0891`) only hashes game-state integers, never rendering artefacts
- **Verify at Phase I:** run 10000 ticks with identical inputs on all 3 platforms and compare hashes

**Contingency:** If a determinism break is found, audit the M11 → M10 boundary for any float-to-int truncation path. There shouldn't be one by design.

### R3: GRAPHICS.DAT Sprite Decoding Edge Cases (Severity: High, Likelihood: Medium)

**Description:** The IMG3 compression format in GRAPHICS.DAT has known edge cases: variable-width entries, local palette overrides, and a nibble-based encoding that's been historically tricky for re-implementations. Fontanel's `GRAPH*.C` files document the algorithm but the PC 3.4 variant may differ from Atari ST in subtle ways (different graphic set entries, different compressed sizes).

**Impact:** Corrupted sprites, visual glitches, or crashes when decoding specific GRAPHICS.DAT entries.

**Mitigation:**
- M10 already has a working IMG3 decoder (`image_backend_pc34_compat.c`) validated against the actual PC 3.4 GRAPHICS.DAT
- Phase F (sprite loading) includes exhaustive tests: decode ALL entries in GRAPHICS.DAT and verify dimensions match header
- Fontanel's `graphics_dat_entry_classify_pc34_compat.h` already classifies entries by type/purpose
- Build a golden-image test suite for the 50 most critical sprites (title screen, first-level walls, common monsters)

**Contingency:** If specific entries crash the decoder, add entry-specific workarounds (skip or substitute a placeholder sprite). Log the entry index for later investigation.

### R4: Audio Format Zoo — What Does PC 3.4 Actually Use? (Severity: Medium, Likelihood: High)

**Description:** DM's audio varies wildly across versions. Atari ST used YM2149 chip sounds. Amiga used Paula samples. PC versions used either Sound Blaster or PC speaker. The exact format stored in (or alongside) DUNGEON.DAT for PC 3.4 is not fully documented. Fontanel's ReDMCSB focuses on Atari ST audio. We need to reverse-engineer the PC 3.4 sound table.

**Impact:** No sound, wrong sounds, or crashes when trying to play audio. Could delay Phase L (audio).

**Mitigation:**
- **Pre-work (before Phase L):** Analyse the PC 3.4 binary for sound-related data. Look for Sound Blaster / AdLib init routines, WAV/VOC file references, or embedded PCM data.
- Fontanel's documentation mentions `SOUND.DAT` or embedded sound table — verify if PC 3.4 has a separate sound data file or if sounds are in GRAPHICS.DAT
- If sounds are not readily available from the PC 3.4 data: extract from Atari ST version and convert (PCM → WAV)
- As last resort: use community-sourced DM sound effects (dmweb.free.fr has shared resources)

**Contingency:** M11 ships without audio if sound format is unresolved. Audio becomes M11.1 patch. The game is playable without sound (the original DM was on systems without sound hardware).

### R5: Apple Code Signing / Notarisation Without Developer Account (Severity: Low, Likelihood: High)

**Description:** macOS Gatekeeper blocks unsigned apps by default. Without a $99/yr Apple Developer account, we can't sign or notarise the .app bundle. Users will need to right-click → Open or run `xattr -cr Firestaff.app` to bypass Gatekeeper.

**Impact:** Friction for macOS users. No impact on functionality.

**Mitigation:**
- Include clear instructions in README and in-DMG README.txt
- Ad-hoc signing (`codesign -s - Firestaff.app`) at least marks the binary as locally signed, which reduces one layer of Gatekeeper friction
- Homebrew cask as an alternative distribution (Homebrew handles quarantine)
- If Daniel has an Apple Developer account: sign and notarise. Adds ~1h to packaging.

**Contingency:** Accept the friction for v0.11. A Developer account can be obtained later for v1.0.

### R6: DPI Scaling on High-Resolution Displays (Severity: Medium, Likelihood: Medium)

**Description:** macOS Retina (2×), Windows at 125%/150%/200%, and Linux HiDPI all report different logical vs physical pixel sizes. A 320×200 game at 2× scale is 640×400 logical — but on a Retina display that's 1280×800 physical pixels. If we scale based on logical pixels, the image is half the expected size. If we scale based on physical pixels, we might exceed display resolution.

**Impact:** Game window appears too small or too large on HiDPI displays. Mouse coordinate mapping is wrong.

**Mitigation:**
- Use `SDL_GetWindowSizeInPixels` (SDL3) to get physical pixel dimensions for scaling calculations
- Calculate scale factor from physical pixels: `scale = min(physW / 320, physH / 200)`
- Mouse coordinates are always in framebuffer space (320×200) — convert from physical pixel space via the scale factor
- Test on Retina (macOS), 150% scaling (Windows), 200% GNOME (Linux)
- Phase B explicitly tests DPI handling

**Contingency:** If DPI is too messy, default to non-DPI-aware mode on problematic platforms (fullscreen works fine regardless).

### R7: Windows Path Separators Breaking `fs_portable_compat` (Severity: Low, Likelihood: Low)

**Description:** Windows uses `\` as path separator. If any code outside `fs_portable_compat` constructs paths with hardcoded `/` and passes them directly to Win32 APIs (bypassing the FSP wrapper), paths will break.

**Impact:** Config/save files not found on Windows.

**Mitigation:**
- **Rule:** ALL file operations go through `fs_portable_compat`. No direct `fopen` in M11 code outside `fs_portable_compat.c` and M10 (M10 handles its own file I/O for DUNGEON.DAT loading).
- The one exception: M10's `fopen` for DUNGEON.DAT. This path is user-supplied and must work as-is. On Windows, `fopen` actually accepts `/` in paths. Verify this.
- `fs_portable_compat.c` converts `/` to `\` internally on Windows before all API calls

**Contingency:** Add a path-fix utility that normalises all `\` to `/` on input and `/` to `\` on output for Windows API calls.

### R8: Asset MD5 Collisions for Legitimate Variant Files (Severity: Low, Likelihood: Low)

**Description:** Different DM versions (EN vs FR vs GE) have different DUNGEON.DAT and GRAPHICS.DAT files with different MD5s. This is expected and intentional. The asset validator must recognise each variant, not just the PC 3.4 EN version.

**Impact:** French or German game files reported as "unrecognised" even though they're valid.

**Mitigation:**
- Maintain a known-hash table with ALL known DM variants (EN, FR, GE, Atari ST, PC, Amiga)
- Unrecognised files still load — the warning is informational, not blocking
- Community can submit new hashes via GitHub issues
- Phase E tests with at least 2 different known files

**Contingency:** If the hash table is incomplete at launch, accept any DUNGEON.DAT that passes header validation (Phase 1 `F0500_DUNGEON_LoadDatHeader_Compat`). MD5 is advisory, not mandatory.

### Risk Summary

| Risk | Severity | Likelihood | Priority |
|------|----------|-----------|----------|
| R3: GRAPHICS.DAT sprite decoding | High | Medium | **1** |
| R4: Audio format uncertainty | Medium | High | **2** |
| R1: SDL3/SDL2 API drift | Medium | Medium | **3** |
| R6: DPI scaling | Medium | Medium | **4** |
| R2: Float determinism via SDL | Low | Low | 5 |
| R5: Apple code signing | Low | High | 6 |
| R7: Windows path separators | Low | Low | 7 |
| R8: Asset MD5 variants | Low | Low | 8 |

**Top 2 risks requiring active mitigation:**
1. **R3 (GRAPHICS.DAT):** Addressed by exhaustive sprite decode testing in Phase F and golden image comparisons in Phase J.
2. **R4 (Audio formats):** Requires pre-Phase-L investigation of PC 3.4 binary. If unresolved, M11 ships without audio.

---

## §9 Acceptance Criteria

M11 is **ACCEPTED** when ALL of the following are true:

### 9.1 Functional Criteria

| # | Criterion | Verification |
|---|-----------|-------------|
| AC01 | Firestaff opens a window on macOS, Linux, and Windows | Manual test on all 3 platforms |
| AC02 | Displays the DM starting screen (or a recognisable rendering of the starting dungeon position) | Screenshot comparison |
| AC03 | Loads DUNGEON.DAT and renders the starting map with correct walls, floor, and ceiling | Golden image test (Phase J) |
| AC04 | Accepts keyboard input to move the party (W/S/A/D/Q/E) | Manual test: party position changes |
| AC05 | Accepts mouse input on compass and UI elements | Manual test: click compass, click inventory |
| AC06 | Plays at least one sound effect when a game event occurs | Manual test: footstep sound on move |
| AC07 | Champion portraits display with health/stamina/mana bars | Screenshot comparison |
| AC08 | Inventory panel opens and shows item sprites | Manual test |
| AC09 | Spell panel renders rune positions | Manual test |
| AC10 | Save (F5) and Load (F9) work correctly | Save, quit, restart, load, verify party position |

### 9.2 Technical Criteria

| # | Criterion | Verification |
|---|-----------|-------------|
| AC11 | All 20 M10 phases still PASS (M11 does not regress M10) | CI: M10 verify script on all platforms |
| AC12 | `firestaff_headless` produces **identical hash** on macOS, Linux, and Windows for same seed+dungeon+ticks | CI: determinism cross-check workflow |
| AC13 | CMake builds succeed with **zero warnings** on all three platforms (Clang, GCC, MSVC) | CI: build workflow |
| AC14 | All M11 probe binaries PASS on all three platforms | CI: M11 verify workflow |
| AC15 | GitHub Actions CI passes on all three platforms | CI dashboard green |
| AC16 | No memory leaks >100 bytes reachable (Valgrind on Linux) | CI: Valgrind run |
| AC17 | Frame render time p99 < 16ms on reference hardware (M1 Mac, modern Linux desktop, Windows 10 desktop) | Local benchmark |

### 9.3 Deliverable Criteria

| # | Criterion | Verification |
|---|-----------|-------------|
| AC18 | macOS .app bundle launches from DMG | Manual test |
| AC19 | Linux AppImage launches on Ubuntu 22.04 | CI or manual test |
| AC20 | Windows NSIS installer installs and exe launches | Manual test |
| AC21 | README updated with M11 build/run instructions | Review |
| AC22 | CHANGELOG.md for v0.11.0 is complete | Review |

### 9.4 Definition of Done

M11 is done when:
- **All 22 acceptance criteria are met**
- **All 132 planned invariants pass**
- **CI is green on all 3 platforms**
- **At least one person has played through the first 3 levels of DM1 using Firestaff** (manual smoke test)

---

## §10 Estimated Effort

### 10.1 Per-Phase Estimates

| Phase | Description | Effort (days) | Cumulative |
|-------|-------------|--------------|------------|
| A | SDL3 Window + Basic Framebuffer | 2.0 | 2.0 |
| B | Scaling + Aspect Ratio | 1.5 | 3.5 |
| C | Filesystem Abstraction | 2.0 | 5.5 |
| D | Config + TOML Parser | 1.5 | 7.0 |
| E | Asset Validation | 1.0 | 8.0 |
| F | Sprite Loading + Cache | 2.0 | 10.0 |
| G | Text Rendering | 1.0 | 11.0 |
| H | Input Handler + Key Bindings | 1.5 | 12.5 |
| I | Main Loop Integration | 2.0 | 14.5 |
| J | Dungeon Viewport Renderer | 5.0 | 19.5 |
| K | UI Panels | 4.0 | 23.5 |
| L | Audio Integration | 2.0 | 25.5 |
| M | CMake Build System | 2.0 | 27.5 |
| N | CI/CD Pipeline | 2.0 | 29.5 |
| O | Packaging | 3.0 | 32.5 |
| P | Integration Testing + Polish | 3.0 | 35.5 |
| **Total** | | **35.5 days** | |

### 10.2 Calendar Estimate

Assuming:
- One developer (AI agent), supervised by Daniel
- ~6 productive hours per day
- Some parallelisation between independent phases (C+D+E with A+B)

**Optimistic:** 5 weeks (with parallelisation, no major blockers)
**Realistic:** 7 weeks (accounting for R3/R4 investigation, cross-platform debugging, feedback iterations)
**Pessimistic:** 10 weeks (if audio format requires deep reverse-engineering, or GRAPHICS.DAT decoding reveals major issues)

### 10.3 Effort Breakdown by Category

| Category | Phases | Days | % of Total |
|----------|--------|------|------------|
| Rendering | A, B, F, G, J, K | 15.5 | 44% |
| Infrastructure | C, D, E, H, I | 8.0 | 23% |
| Audio | L | 2.0 | 6% |
| Build/CI/Packaging | M, N, O | 7.0 | 20% |
| Integration/Polish | P | 3.0 | 8% |

Rendering dominates — as expected. The dungeon viewport (Phase J, 5 days) is the single largest effort. This is where the game goes from "technical demo" to "recognisably Dungeon Master".

### 10.4 Pre-Work Required Before Phase A

Before starting Phase A implementation, the following must be verified or prepared:

1. **GRAPHICS.DAT location confirmed:** `$HOME/.openclaw/data/redmcsb-original/GRAPHICS.DAT` — verify it exists and is the PC 3.4 EN version.
2. **SDL3 available on build machine:** `brew install sdl3 sdl3_mixer` on macOS.
3. **Audio format investigation (for R4):** Spend 2-4 hours examining the PC 3.4 binary or data files for sound format clues. Document findings. This doesn't block Phase A but must be done before Phase L.
4. **GRAPHICS.DAT entry map:** Build or verify a table mapping GRAPHICS.DAT entry indices to their purpose (wall segments, sprites, UI elements, font). Fontanel's `graphics_dat_entry_classify_pc34_compat.h` is a starting point.

### 10.5 Cost Summary

| Metric | Value |
|--------|-------|
| **New modules** | 12 |
| **New LOC (estimated)** | ~11 000 |
| **New probe files** | 16 |
| **New invariants** | 132 |
| **Calendar (realistic)** | 7 weeks |
| **Developer-days** | 35.5 |
| **Risk items** | 8 (2 high-priority) |
| **Acceptance criteria** | 22 |

---

## Appendix A: Glossary

| Term | Meaning |
|------|---------|
| **M10** | Milestone 10 — the deterministic engine core (20 phases, 500+ invariants) |
| **M11** | Milestone 11 — the SDL3 presentation layer (this plan) |
| **Phase** | A self-contained implementation step with its own probe and invariants |
| **Probe** | A test binary that verifies invariants for a specific phase |
| **Invariant** | A verifiable assertion that must hold true on every build/run |
| **Golden image** | A reference PNG that the rendered framebuffer must match pixel-for-pixel |
| **Tick** | One game-logic step (~166ms in real time, ≈6 Hz) |
| **Frame** | One display refresh (~16ms at 60fps) — multiple frames per tick |
| **Framebuffer** | The internal 320×200 8-bit indexed colour buffer |
| **Emission** | An event notification from the tick orchestrator (sound, damage, etc.) |
| **DM** | Dungeon Master (1987) |
| **CSB** | Chaos Strikes Back (1989) |
| **PC 3.4** | The PC DOS version 3.4 (English) — our baseline |
| **Fontanel** | Christophe Fontanel, author of the ReDMCSB reverse-engineering project |
| **IMG3** | The sprite compression format used in GRAPHICS.DAT |
| **VGA DAC** | The 6-bit colour registers of the VGA display adapter |
| **LCG** | Linear Congruential Generator — the Borland-compatible RNG used by M10 |
| **LSB-first** | Least Significant Byte first (little-endian) — the serialisation order |

---

## Appendix B: File Naming Convention

M10 files use `*_pc34_compat.{h,c}` suffix. M11 files use `*_m11.{h,c}` suffix, except for cross-milestone modules (`fs_portable_compat`, `asset_validator_compat`, `md5_compat`) which use `*_compat` without the `_m11` tag — these are designed to be shared across milestones.

**Probe naming:** `firestaff_m11_<phasename>_probe.c`

**Golden image naming:** `golden/m11_<scenario>_<facing>.png`

---

## Appendix C: SDL3 vs SDL2 Compatibility Cheat Sheet

| SDL2 | SDL3 | Notes |
|------|------|-------|
| `SDL_Init(SDL_INIT_VIDEO)` | `SDL_Init(SDL_INIT_VIDEO)` | Same |
| `SDL_CreateWindow(title, x, y, w, h, flags)` | `SDL_CreateWindow(title, w, h, flags)` | SDL3 drops x,y params |
| `SDL_CreateRenderer(win, -1, flags)` | `SDL_CreateRenderer(win, NULL)` | SDL3 uses driver name string |
| `SDL_RenderCopy(r, tex, src, dst)` | `SDL_RenderTexture(r, tex, src, dst)` | Renamed |
| `SDL_RenderPresent(r)` | `SDL_RenderPresent(r)` | Same |
| `SDL_GetWindowSize(win, &w, &h)` | `SDL_GetWindowSize(win, &w, &h)` | Same (logical) |
| N/A | `SDL_GetWindowSizeInPixels(win, &w, &h)` | Physical pixels (new in SDL3) |
| `SDL_WINDOWEVENT` | `SDL_EVENT_WINDOW_*` | Events restructured |
| `Mix_OpenAudio(freq, fmt, ch, chunk)` | `Mix_OpenAudio(0, NULL)` | SDL3_mixer simplified |

---

*End of M11 Plan. Document version 1.0, 2026-04-20.*