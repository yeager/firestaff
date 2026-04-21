# M11 — Remaining Work to Practical Completion

**Updated:** 2026-04-21
**Baseline:** 115/115 game-view invariants + 18/18 Phase-A invariants passing
**Status:** ~70% complete — core gameplay loop works, graphics pipeline works, remaining work is audio, CMake, CI, packaging, and polish

---

## §1  Current Verified Baseline

### What is already done (verified by probes)

| Area | Status | Probe evidence |
|------|--------|---------------|
| **SDL3 window + framebuffer** | ✅ Done | 18/18 Phase-A invariants (scaling, palette, letterbox, HiDPI mapping) |
| **GRAPHICS.DAT asset loader** | ✅ Done | INV_GV_84–96 (init, enumerate, load, cache, blit, scale, region extract) |
| **VGA palette** | ✅ Done | `vga_palette_pc34_compat.{h,c}` + `test_vga_palette` binary |
| **Dungeon viewport rendering** | ✅ Done | Real textures from GRAPHICS.DAT, wall/floor blitting, depth-correct sprites, layered face bands |
| **Movement (WASD + strafe + click)** | ✅ Done | INV_GV_04–06, 07K–L (relative strafe, click navigation) |
| **Turn (QE + click)** | ✅ Done | INV_GV_04–05, 07F |
| **Front-cell inspection** | ✅ Done | INV_GV_07, 07D (keyboard + mouse inspect) |
| **Champion cycling** | ✅ Done | INV_GV_07B (Tab) |
| **Combat (melee attack)** | ✅ Done | INV_GV_07C, 07H (Space/click → real strike tick) |
| **Door interaction** | ✅ Done | INV_GV_07I (Space toggles door open/closed) |
| **Auto-clock advance** | ✅ Done | INV_GV_07J (idle cadence drives real world clock) |
| **Item pickup/drop** | ✅ Done | INV_GV_27–32 (G/P keys, HandleInput routing) |
| **Creature AI** | ✅ Done | INV_GV_33–38 (movement, damage, message log, sight range, dead handling) |
| **Pit fall + teleporters** | ✅ Done | INV_GV_39–48 (level transitions, damage, coord preservation, rotation) |
| **Spell casting UI** | ✅ Done | INV_GV_49–60 (panel open/close, rune entry, clear, cast, encoding) |
| **Spell effect pipeline** | ✅ Done | INV_GV_61–65 (Light, Fireball, Party Shield, state deltas, emissions) |
| **Stair transitions (up/down)** | ✅ Done | INV_GV_66–69 (descent, ascent, boundary, log) |
| **XP/leveling** | ✅ Done | INV_GV_70–76 (skill query, combat XP, magic XP, kill XP) |
| **Potion/flask/food use** | ✅ Done | INV_GV_77–83 (heal, poison, flask conversion, slot clearing) |
| **Survival (food drain, death)** | ✅ Done | INV_GV_25–26 |
| **Rest toggle** | ✅ Done | INV_GV_22–22B |
| **Message log** | ✅ Done | INV_GV_21, 24 |
| **Quicksave/quickload** | ✅ Done | INV_GV_13B–13E |
| **Sidebar HUD (save/load/menu)** | ✅ Done | INV_GV_13D–F, 14 |
| **Party status strip** | ✅ Done | INV_GV_15–15B |
| **Forward depth chips** | ✅ Done | INV_GV_20 |
| **Near-lane scanner** | ✅ Done | INV_GV_18 |
| **Action feedback overlay** | ✅ Done | INV_GV_19 |
| **Minimap inset** | ✅ Done | INV_GV_08, 12 |
| **Menu ↔ game transition** | ✅ Done | INV_GV_01–02, 13 |
| **Viewport feature cues** | ✅ Done | INV_GV_10–12B (doors, stairs, items, effects in viewport) |

### What is NOT done

| Area | Status | Notes |
|------|--------|-------|
| **Audio** | ❌ Not started | No SDL3_mixer integration. Sound emissions are recognized but silently consumed. |
| **CMake build system** | ❌ Not started | Build uses shell scripts only. No CMakeLists.txt exists. |
| **CI build on Linux/Windows** | ❌ Not started | CI only runs M10 probes. M11 probes are macOS-only shell scripts. |
| **Cross-platform filesystem** | 🟡 Partial | `fs_portable_compat` is spec'd but not a standalone module; paths are handled ad-hoc via `FIRESTAFF_DATA` env var. |
| **Configurable key bindings** | 🟡 Partial | Keys are hardcoded in probe/game-view. No runtime rebinding UI. Config stores some values but doesn't drive bindings. |
| **Inventory UI panel** | 🟡 Partial | Items show in party strip. No full grid-based inventory with drag-drop. |
| **Map overlay** | 🟡 Partial | Minimap inset exists. No full-screen toggleable map. |
| **Packaging (DMG/AppImage/NSIS)** | ❌ Not started | |
| **Memory leak audit** | ❌ Not started | |
| **Performance profiling** | ❌ Not started | |
| **Golden image regression** | ❌ Not started | Screenshots exist but no automated comparison. |
| **Cross-platform determinism check** | ❌ Not started | Only macOS tested. |

---

## §2  Remaining Scope

### What constitutes M11 "practically complete"

M11 is done when a human can:
1. Launch `firestaff` on macOS (minimum), ideally Linux/Windows too
2. See real DM textures in the viewport (✅ already works)
3. Navigate, fight, cast spells, use items, rest, save/load (✅ already works)
4. Hear at least basic sound effects when events occur
5. Build the project with CMake (not just shell scripts)
6. CI proves it works on at least 2 platforms

### What can be deferred to M12 or later

- Full inventory drag-drop UI → M12
- Full-screen map overlay → M12
- Runtime key rebinding UI → M12 (settings panel)
- Packaging (DMG/AppImage/NSIS) → M12 or post-M12
- Windows build → best-effort in M11, full support M12
- Cross-platform determinism CI → M12
- Memory leak audit → M12

---

## §3  Execution Roadmap (Ordered Slices)

### Slice 1: Audio Integration (Priority: HIGH)

**Goal:** SDL3_mixer initialization and sound playback from tick emissions.

**Files involved:**
- `audio_sdl_m11.h` (new)
- `audio_sdl_m11.c` (new)
- `main_loop_m11.c` (wire audio init/shutdown)
- `m11_game_view.c` (route EMIT_SOUND_REQUEST to audio module)

**Implementation notes:**
- M10 already emits `EMIT_SOUND_REQUEST` in `TickResult_Compat.emissions[]` — the game view already recognizes these (see `m11_tick_has_emission_kind`).
- PC 3.4 audio is the biggest unknown. Start with a **synthetic placeholder approach**: map emission event types to built-in procedural beeps/clicks via SDL3_mixer, or ship 10-15 small WAV files for key events (footstep, door, hit, miss, spell, death).
- If original PC 3.4 sounds can be extracted from the data files, add that path. Otherwise, placeholder sounds are acceptable for M11.
- Volume is read from `config_m12.c` (master_volume already stored).

**Invariants to add (8):**
- `INV_AU_01`: SDL3_mixer initializes without error
- `INV_AU_02`: Playing a test WAV succeeds
- `INV_AU_03`: EMIT_SOUND_REQUEST footstep → audio channel fires
- `INV_AU_04`: EMIT_SOUND_REQUEST door → audio channel fires
- `INV_AU_05`: Volume 0.0 produces no audible output (channel volume = 0)
- `INV_AU_06`: Volume 1.0 sets max channel volume
- `INV_AU_07`: Mute/unmute toggle works
- `INV_AU_08`: 10 simultaneous sounds don't crash

**Risks:**
- SDL3_mixer availability on build machine → `brew install sdl3_mixer`
- Original sounds unknown → mitigate with placeholder WAVs

**Verification:**
```bash
# Build with SDL3_mixer linked
cc -std=c99 -Wall -O2 -I. -o firestaff_m11_audio_probe_bin \
    firestaff_m11_audio_probe.c audio_sdl_m11.c \
    $(sdl3-config --cflags --libs) -lSDL3_mixer
./firestaff_m11_audio_probe_bin
```

**Effort:** 3 days

---

### Slice 2: CMake Build System (Priority: HIGH)

**Goal:** Replace shell-script builds with CMake for reproducible cross-platform compilation.

**Files involved:**
- `CMakeLists.txt` (new, top-level)
- `cmake/FindSDL3.cmake` (new, if needed)
- `cmake/CompilerWarnings.cmake` (new)

**Implementation notes:**
- The flat directory layout stays. CMake globs `*_pc34_compat.c` for M10, lists M11/M12 files explicitly.
- Two main targets: `firestaff` (SDL3-linked) and `firestaff_headless` (M10-only).
- Probe targets built from `firestaff_m11_*_probe.c` and `firestaff_m12_*_probe.c`.
- SDL3 detection via `find_package(SDL3)`. SDL2 fallback optional.
- Compiler warnings: `-Wall -Wextra -Wpedantic` on GCC/Clang, `/W4` on MSVC.
- Install rules for binary + man page.

**Invariants to add (6):**
- `INV_CM_01`: `cmake -B build` configures without error on macOS
- `INV_CM_02`: `cmake --build build` compiles all targets with zero errors
- `INV_CM_03`: `firestaff_headless` runs M10 probes successfully
- `INV_CM_04`: `firestaff_m11_phase_a_probe_bin` links and passes
- `INV_CM_05`: `firestaff_m11_game_view_probe_bin` links and passes
- `INV_CM_06`: `firestaff_m12_startup_menu_probe_bin` links and passes

**Risks:**
- SDL3 CMake module not found → ship FindSDL3.cmake or use pkg-config fallback
- M10 headers use COMPILE_H guard macro → pass via `target_compile_definitions`

**Verification:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && ctest --output-on-failure
```

**Effort:** 2 days

---

### Slice 3: CI Pipeline for M11/M12 Probes (Priority: MEDIUM)

**Goal:** GitHub Actions workflow that builds and runs all probes on macOS + Linux.

**Files involved:**
- `.github/workflows/verify-m11.yml` (new)
- `.github/workflows/verify-m12.yml` (new or merged)

**Implementation notes:**
- macOS runner: `macos-14` (Apple Silicon). Install SDL3 via brew.
- Linux runner: `ubuntu-24.04`. Install SDL3 from PPA or build from source. For headless probes (no actual window needed), use `SDL_VIDEODRIVER=dummy`.
- Windows: best-effort, can be added later.
- M11 probes need `FIRESTAFF_DATA` set to a directory with DUNGEON.DAT/GRAPHICS.DAT — either download from a private artifact or skip asset-dependent invariants in CI.
- Strategy: split probes into "asset-dependent" (game-view) and "asset-independent" (phase-a, config, startup-menu) and gate the former on asset availability.

**Invariants to add (4):**
- `INV_CI_01`: macOS build + headless probes pass
- `INV_CI_02`: Linux build + headless probes pass
- `INV_CI_03`: M10 verify script still passes alongside M11
- `INV_CI_04`: All probe log files uploaded as artifacts

**Risks:**
- Game data files can't be in public CI → run asset-independent probes only, or use encrypted artifacts
- SDL3 on Ubuntu → may need to build from source (not yet in apt)

**Effort:** 2 days

---

### Slice 4: Filesystem Abstraction Cleanup (Priority: MEDIUM)

**Goal:** Extract the ad-hoc path handling into a proper `fs_portable_compat` module.

**Files involved:**
- `fs_portable_compat.h` (new)
- `fs_portable_compat.c` (new)
- `config_m12.c` (refactor to use fs_portable for path resolution)
- `m11_game_view.c` (refactor FIRESTAFF_DATA lookups)
- `asset_loader_m11.c` (refactor path joins)

**Implementation notes:**
- Current state: paths are handled via `getenv("FIRESTAFF_DATA")` and string concatenation. This works on macOS but won't on Windows.
- Implement: `FSP_GetUserDataDir`, `FSP_JoinPath`, `FSP_PathExists`, `FSP_CreateDirectoryRecursive`.
- macOS: `~/Library/Application Support/Firestaff/`
- Linux: `$XDG_DATA_HOME/firestaff/`
- Windows: `%APPDATA%\Firestaff\`
- Auto-discover game data: look in user data dir, then `FIRESTAFF_DATA` env, then current directory.

**Invariants to add (6):**
- `INV_FS_01`: GetUserDataDir returns non-empty path
- `INV_FS_02`: JoinPath("a","b") produces "a/b"
- `INV_FS_03`: CreateDirectoryRecursive creates nested dirs
- `INV_FS_04`: PathExists returns true for created dir
- `INV_FS_05`: Read/write file round-trips correctly
- `INV_FS_06`: NormalizePath collapses ".." segments

**Effort:** 2 days

---

### Slice 5: Full-Screen Inventory Panel (Priority: LOW)

**Goal:** Grid-based inventory view for the selected champion, replacing the compact party-strip display.

**Files involved:**
- `m11_game_view.c` (add inventory rendering functions)
- `m11_game_view.h` (add inventory state fields)

**Implementation notes:**
- Toggle with `I` key or clicking a champion portrait.
- Shows: head, neck, torso, legs, feet, pouch ×2, quiver ×4, backpack ×3 slots.
- Item sprites from `M11_AssetLoader`.
- Click to select item for use/equip. No drag-drop yet (deferred to M12).
- Read from `ChampionState_Compat.inventory[]` (M10 Phase 10 data).

**Invariants to add (5):**
- `INV_IV_01`: I key toggles inventory panel visibility
- `INV_IV_02`: Inventory renders all slot types for active champion
- `INV_IV_03`: Occupied slot shows item sprite
- `INV_IV_04`: Empty slot shows outline
- `INV_IV_05`: Clicking slot with item triggers use-item path

**Effort:** 3 days

---

### Slice 6: Full-Screen Map Overlay (Priority: LOW)

**Goal:** Toggleable full-screen map showing explored dungeon tiles.

**Files involved:**
- `m11_game_view.c` (add map rendering)

**Implementation notes:**
- Toggle with `M` key.
- Uses `exploredBits[]` already tracked in `M11_GameViewState`.
- Top-down view of current level: walls as lines, doors as gaps, party as arrow.
- Read tile types from `GameWorld_Compat.dungeonDat`.

**Invariants to add (3):**
- `INV_MAP_01`: M key toggles map overlay
- `INV_MAP_02`: Map renders explored tiles, unexplored tiles are dark
- `INV_MAP_03`: Party position and facing shown on map

**Effort:** 2 days

---

### Slice 7: Integration Polish (Priority: MEDIUM)

**Goal:** End-to-end stability, README update, golden image baseline.

**Files involved:**
- `README.md` (update with M11 build/play instructions)
- Various probes (add golden image comparison)
- `m11_game_view.c` (bug fixes from playtesting)

**Implementation notes:**
- Manual playthrough of first 3 DM1 levels.
- Fix any crashes or rendering glitches found.
- Capture golden image PNGs for starting position viewport.
- Add `firestaff_m11_golden_image_probe.c` that compares rendered framebuffer to reference.
- Update README with: what M11 is, how to build (CMake), how to run, what works.

**Invariants to add (4):**
- `INV_PL_01`: 200-tick walkthrough completes without crash
- `INV_PL_02`: Starting position viewport matches golden image (within tolerance)
- `INV_PL_03`: All M10 probes still pass
- `INV_PL_04`: README build instructions work from clean checkout

**Effort:** 3 days

---

## §4  Recommended Execution Order

```
Slice 2 (CMake)          ← do first, unlocks CI and cross-platform
  ↓
Slice 3 (CI)             ← proves CMake works on multiple platforms
  ↓
Slice 4 (FS abstraction) ← needed for Windows, cleans up paths
  ↓
Slice 1 (Audio)          ← biggest missing user-facing feature
  ↓
Slice 5 (Inventory)      ← gameplay completeness
  ↓
Slice 6 (Map)            ← gameplay completeness
  ↓
Slice 7 (Polish)         ← final validation
```

**Critical path:** Slices 2→3→1→7 (CMake → CI → Audio → Polish) = ~10 days
**Full path including optional:** ~17 days

---

## §5  Per-Slice Codex Implementation Briefs

### Codex Prompt: Slice 2 (CMake)

```
In the firestaff project (tmp/firestaff/), create a CMakeLists.txt that:

1. Sets cmake_minimum_required(VERSION 3.20) and project(Firestaff VERSION 0.11.0 LANGUAGES C)
2. Sets C standard to C11
3. Collects all *_pc34_compat.c files into a static library `firestaff_m10`
   - These need compile definitions: COMPILE_H, STATICFUNCTION=static, SEPARATOR=comma, FINAL_SEPARATOR=close-paren
4. Collects M11 sources (main_loop_m11.c, m11_game_view.c, render_sdl_m11.c, asset_loader_m11.c, firestaff_main_m11.c) into firestaff_m11 static lib
5. Collects M12 sources (menu_startup_m12.c, config_m12.c, asset_status_m12.c, card_art_m12.c, branding_logo_m12.c) into firestaff_m12 static lib
6. Finds SDL3 via find_package
7. Creates firestaff executable linking m12 + m11 + m10 + SDL3
8. Creates probe executables from firestaff_m11_*_probe.c and firestaff_m12_*_probe.c
9. Adds -Wall -Wextra on GCC/Clang, /W4 on MSVC
10. All source files are in the same flat directory (no subdirectories)

The M10 compat headers use special macros. Pass them via target_compile_definitions on firestaff_m10:
  COMPILE_H STATICFUNCTION=static "SEPARATOR=," "FINAL_SEPARATOR=)"

Verify: cmake -B build && cmake --build build should produce firestaff and all probe binaries.
```

### Codex Prompt: Slice 1 (Audio)

```
In the firestaff project (tmp/firestaff/), create audio_sdl_m11.h and audio_sdl_m11.c:

1. Use SDL3_mixer (SDL3/SDL_mixer.h) for audio playback
2. Initialize with Mix_OpenAudio(0, NULL) — SDL3_mixer signature
3. Map EMIT_SOUND_REQUEST emission payloads to sound channels:
   - For now, generate simple procedural sounds or load placeholder WAVs
   - Sound categories: footstep, door, hit, miss, spell, death, pickup, drop
4. Provide M11_Audio_Init(), M11_Audio_Shutdown(), M11_Audio_ProcessEmissions(TickResult_Compat*)
5. Volume control: M11_Audio_SetMasterVolume(float 0.0-1.0), M11_Audio_ToggleMute()
6. Safe to call ProcessEmissions every tick — if no SOUND_REQUEST emissions, does nothing

Wire into m11_game_view.c: after each F0884_ORCH_AdvanceOneTick_Compat call, pass the TickResult to M11_Audio_ProcessEmissions.

The game view already has: m11_tick_has_emission_kind(&state->lastTickResult, EMIT_SOUND_REQUEST)

Create firestaff_m11_audio_probe.c with 8 invariants testing init, play, volume, mute, concurrent sounds.

Existing sound emission infrastructure in memory_tick_orchestrator_pc34_compat.h:
  EMIT_SOUND_REQUEST is already a valid emission kind
  TickResult_Compat.emissions[].kind, .payload fields carry the sound event type
```

---

## §6  Risks and Deferrals

| Risk | Severity | Mitigation |
|------|----------|------------|
| SDL3_mixer not available on Linux CI runners | Medium | Build SDL3 from source in CI, or skip audio probe |
| Original PC 3.4 sounds unknown format | Medium | Ship placeholder WAVs; add real extraction later |
| Windows path separator issues | Low | fs_portable_compat handles conversion |
| CMake can't find M10 headers with COMPILE_H macro | Low | Pass defines via target_compile_definitions |
| Game data files can't be in public CI | Medium | Run asset-dependent probes only on self-hosted runner or skip |

**Deferred to M12:**
- Full key rebinding UI
- Full inventory drag-drop
- Packaging (DMG/AppImage/NSIS)
- Cross-platform determinism CI
- Memory leak audit (Valgrind)
- Performance profiling

---

## §7  Verification Strategy

All probes follow the existing pattern:
1. Shell script compiles probe + dependencies
2. Runs binary, captures log
3. Checks `# summary: N/N invariants passed`
4. Exit 1 on any failure

CMake adds a `ctest` path:
```bash
cmake -B build
cmake --build build
cd build && ctest --output-on-failure
```

CI workflow uploads probe logs as artifacts for debugging.

---

*End of M11 Remaining Plan. Version 2.0, 2026-04-21.*
