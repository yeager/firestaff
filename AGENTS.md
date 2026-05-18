# AGENTS.md — Firestaff Project Guide

## What Is Firestaff?

Source-faithful Dungeon Master engine for modern hardware. Plays DM1, Chaos Strikes Back (CSB), DM2: Skullkeep, and DM Nexus (Saturn) with original fidelity or enhanced visuals.

**Repo:** https://github.com/yeager/firestaff
**License:** MIT
**Language:** C (pure C11, no C++)
**Build:** CMake + SDL3
**Platforms:** macOS (Apple Silicon + Intel), Windows, Linux x86_64, Linux ARM64

## Architecture

### Layer Model

```
M12 — Modern launcher UI (1920×1080 HD canvas)
  ↓ launch
M11 — Game engine: rendering, input, game view, game loop
  ↓ delegates to
M10 — Data layer: dungeon, graphics, memory, tick orchestrator
  ↓ reads
Original game files (GRAPHICS.DAT, DUNGEON.DAT, etc.)
```

### Source Directories

| Directory | Purpose |
|-----------|---------|
| `src/engine/` | Main loop, game view (21K LOC), render pipeline, SDL integration |
| `src/ui/` | M12 launcher: menu logic, hit-testing, modern HD rendering |
| `src/memory/` | M10 data layer: dungeon, movement, combat, sensors, timeline, savegame |
| `src/shared/` | Cross-game: asset loading, palette, VGA compat, config, touch zones |
| `src/frontend/` | Title screens, entrance sequences, V1 chrome rendering |
| `src/dm1/` | DM1-specific: spell casting, skill/XP, collision, viewport, movement pipeline |
| `src/dm1v2/` | DM1 V2 graphics mode (filtered, upscaled, modern) |
| `src/csb/` | Chaos Strikes Back: game state, dungeon loader, chaos magic |
| `src/dm2/` | DM2 Skullkeep: game state, dungeon loader |
| `src/nexus/` | DM Nexus (Saturn): DGN level format, DMDF parser |
| `src/test/` | Test utilities |
| `include/` | All public headers (~365 files) |
| `tests/` | Integration tests (~149 files) |
| `probes/` | Headless verification probes (Phase A = CI, others = local) |

### Key Files

| File | What It Does |
|------|-------------|
| `src/engine/m11_game_view.c` | Game view: rendering, input dispatch, HUD, dialog overlays (21K LOC) |
| `src/engine/main_loop_m11.c` | SDL event loop, menu↔game transitions, mouse mapping |
| `src/engine/firestaff_game_loop.c` | Asset loading, game tick, V1 command processing |
| `src/ui/menu_startup_m12.c` | Launcher state machine: navigation, game options, launch logic |
| `src/ui/menu_startup_render_modern_m12.c` | Launcher HD renderer (1920×1080 canvas) |
| `src/ui/menu_hit_m12.c` | Launcher mouse hit-testing |
| `src/shared/asset_status_m12.c` | Game version catalog: hash-verified GRAPHICS.DAT/DUNGEON.DAT |
| `src/shared/changelog_m12.c` | Version string and changelog text |
| `src/engine/firestaff_accessibility.c` | Accessibility manifest (JSON, atomic writes) |

### Graphics Modes (V1/V2)

| Mode | Resolution | Description |
|------|-----------|-------------|
| V1 Original | 320×200 | Pixel-perfect original |
| V2.0 Filtered | 320×200 + post | CRT scanlines, palette correction |
| V2.1 Upscaled | 3200×2000 | 10× AI upscale |
| V2.2 Modern | 1920×1080 | New 3D-rendered 2D art |

### Asset Discovery

Firestaff finds game files by **MD5 hash**, not filename or path. Place original `GRAPHICS.DAT` and `DUNGEON.DAT` anywhere under `~/.firestaff/data/` and Firestaff will find them.

Default data directory: `~/.firestaff/data/`
Subdirectories: `dm1/`, `csb/`, `dm2/`, `nexus/`, `dm1-multilingual/`

### Supported Game Versions (hash-verified)

**DM1:** PC 3.4 English, PC 3.4 Multilanguage
**CSB:** PC 3.4 English, Atari ST 2.0/2.1, Amiga 3.5, Amiga 3.5 Multilanguage
**DM2:** PC English, PC French, PC German/English JewelCase
**Nexus:** Saturn DMDF/DGN format (138 files)

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Requires SDL3. On macOS: `brew install sdl3`

### CI Workflows

| Workflow | Trigger | What |
|----------|---------|------|
| `verify.yml` | push to main/develop, PRs | CMake build + Phase A probe on macOS-14 |
| `release.yml` | tag `v*` or manual dispatch | Build + package for all platforms |
| `pages.yml` | push to main | Deploy docs to GitHub Pages |

### Release Packaging

Scripts in `scripts/`:
- `package_macos_preview.sh` — DMG + ZIP for macOS
- `package_windows_preview.sh` — ZIP for Windows
- `package_windows_installer_preview.sh` — EXE installer
- `package_linux_preview.sh` — DEB + RPM (x86_64 and ARM64)

## Source Lock: ReDMCSB

Every game system is cross-referenced against the ReDMCSB decompilation:
- **Source:** http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z
- **Local:** `~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/`
- **Key path:** `Toolchains/Common/Source/` (DUNGEON.C, COMMAND.C, ENTRANCE.C, etc.)

**Rule:** Always consult ReDMCSB source before implementing or fixing game logic. Cite relevant source files, functions, and line numbers in comments.

Secondary references:
- **CSBWin:** https://github.com/BeipDev/CSBWin (champion/resurrect/mouse routing)
- **CSB lineage:** https://github.com/zelurker/CSB (source under `src/`)
- **Greatstone:** http://greatstone.free.fr/dm/g_dm.html (dungeon maps, graphics atlas)

## Version Management

Version must be synchronized in three places:
1. `CMakeLists.txt` — `project(Firestaff VERSION x.y.z)`
2. `src/ui/menu_startup_m12.c` — `#define FIRESTAFF_VERSION_STRING`
3. `src/shared/changelog_m12.c` — `M12_Changelog_VersionString()`

Release tags: `v1.7.3`, `v1.7.2`, etc. The CMake version should match the latest release.

## Testing

- **Phase A probe:** `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` — headless, no game data needed. Runs in CI.
- **Integration tests:** `./build/test_*` — individual test binaries. Most need game data.
- **Verification scripts:** `scripts/verify_*.py` — Python scripts that check source-lock invariants.

## Conventions

- All game-specific code is suffixed `_pc34_compat` (PC 3.4 compatibility layer)
- Headers live in `include/`, never alongside `.c` files
- Comments cite ReDMCSB functions: `/* ReDMCSB: COMMAND.C F0359 line ~120 */`
- Commits reference pass numbers when applicable: `pass602b`, `BUG-007`, etc.
- Subagents commit but NEVER push. Main verifies before push.
- No API keys, tokens, passwords, or secrets in any file. Game data files stay user-supplied.

## Common Tasks

### Adding a new game system
1. Read the relevant ReDMCSB source files
2. Create header in `include/` and implementation in `src/<game>/`
3. Add to CMakeLists.txt source list
4. Write integration test in `tests/`
5. Build and verify: `cmake --build build && ./build/test_<name>`

### Fixing a bug
1. Identify the ReDMCSB source reference
2. Read the current implementation
3. Fix with source-lock citation in comments
4. Build, test, commit with descriptive message
5. Do NOT push — main session verifies first

### Updating version for release
1. Update `CMakeLists.txt` project VERSION
2. Update `FIRESTAFF_VERSION_STRING` in `src/ui/menu_startup_m12.c`
3. Update `M12_Changelog_VersionString()` in `src/shared/changelog_m12.c`
4. Update `RELEASE_NOTES.md`
5. Commit, push, tag with `vX.Y.Z`

## Project Stats

- **307 source files**, **245K+ lines of C**
- **365 headers**, **149 tests**, **80+ probes**
- **297/304 tests passing** (CI)
- **20 languages** supported in the launcher
- **4 games:** DM1, CSB, DM2, DM Nexus
