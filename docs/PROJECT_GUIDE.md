# Firestaff project guide

Reference navigation, not mandatory reading for every edit. Descriptive
snapshots may be outdated; current code and verification evidence take precedence.

Contributor requirements and verification/publication policy live in
[AGENTS.md](../AGENTS.md). See also [archived project notes](PROJECT_NOTES_HISTORICAL.md).

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
| `src/engine/` | Main loop, game view, render pipeline, SDL integration |
| `src/ui/` | M12 launcher: menu logic, hit-testing, modern HD rendering |
| `src/memory/` | M10 data layer: dungeon, movement, combat, sensors, timeline, savegame |
| `src/shared/` | Cross-game: asset loading, palette, VGA compat, config, touch zones |
| `src/frontend/` | Title screens, entrance sequences, V1 chrome rendering |
| `src/dm1/` | DM1-specific: spell casting, skill/XP, collision, viewport, movement pipeline |
| `src/dm1v2/` | DM1 V2 graphics mode (filtered, upscaled, modern) |
| `src/csb/` | Chaos Strikes Back: game state, dungeon loader, chaos magic |
| `src/dm2/` | DM2 Skullkeep: game state, dungeon loader |
| `src/nexus/` | DM Nexus (Saturn): DGN level format, DMDF parser |
| `src/theron/` | Theron's Quest: profile, data, rendering, mechanics, progression |
| `src/test/` | Test utilities |
| `include/` | Public headers |
| `tests/` | Test sources |
| `probes/` | Headless verification probes (Phase A = CI, others = local) |
| `verification-screens/` | Tracked project screenshots suitable for README/public docs |
| `docs/compare/` | Tracked visual comparison assets suitable for public docs |

### Key Files

| File | What It Does |
|------|-------------|
| `src/engine/m11_game_view.c` | Game view: rendering, input dispatch, HUD, dialog overlays |
| `src/engine/main_loop_m11.c` | SDL event loop, menu↔game transitions, mouse mapping |
| `src/engine/firestaff_game_loop.c` | Asset loading, game tick, V1 command processing |
| `src/ui/menu_startup_m12.c` | Launcher state machine: navigation, game options, launch logic |
| `src/ui/menu_startup_render_modern_m12.c` | Launcher HD renderer (1920×1080 canvas) |
| `src/ui/menu_hit_m12.c` | Launcher mouse hit-testing |
| `src/shared/asset_status_m12.c` | Game version catalog and recursive hash-verified game-data scanner |
| `src/shared/changelog_m12.c` | Version string and changelog text |
| `src/engine/firestaff_accessibility.c` | Accessibility manifest (JSON, atomic writes) |
| `TODO.md` | Open work and remaining verification gaps |
| `DONE.md` | Verified completed work |

### Graphics Modes (V1/V2)

| Mode | Resolution | Description |
|------|-----------|-------------|
| V1 Original | 320×200 | Pixel-perfect original |
| V2.0 Filtered | 320×200 + post | CRT scanlines, palette correction |
| V2.1 Upscaled | 3200×2000 | 10× AI upscale |
| V2.2 Modern | 1920×1080 | New 3D-rendered 2D art |

### Asset Discovery and Launch Gating

Firestaff finds game files by **hash**, not filename or path. The configured data root is searched recursively, so users may keep their own folder layout.

ZIP archives and ISO/BIN disc images are valid game-data containers. The scanner hashes entries/files inside those containers and reports matches as virtual paths such as `archive.zip::GRAPHICS.DAT` or `disc.iso::DUNGEON.DAT`. ZIP support covers stored entries everywhere and deflated entries when zlib is available at build time. ISO/BIN support is intended for ISO 9660 data images, especially DM2 disc images and the existing Saturn/Nexus path. Archive-backed required files are read directly from their original container into bounded process memory; Firestaff must not extract or cache game data on disk before launch.

Default data directory: `~/.firestaff/data/`
Suggested subdirectories: `dm1/`, `csb/`, `dm2/`, `nexus/`, `theron/`, `dm1-multilingual/`

The data root must be configurable from the start menu. The CLI also supports:

```bash
firestaff --scan-data
firestaff --scan-game-data
firestaff --data-dir ~/Games/FirestaffData --scan-data
```

The start menu automatically scans game data, shows availability for each game, and displays an OK popup when no game data is found or when a selected game is missing required files.

Required game data must block launch:

- DM1/CSB/DM2 require their required GRAPHICS and DUNGEON hashes.
- Nexus/Theron require their primary hash markers.
- Optional title, intro, FTL logo, and other non-essential extras may be skipped when absent.

### Supported Game Versions (hash-verified)

**DM1:** PC 3.4 English, PC 3.4 Multilanguage
**CSB:** Atari ST 2.0/2.1, Amiga 3.1/3.5, FM Towns English/Japanese. CSBWin/ReDMCSB are reference lineages, not Windows/DOS game editions.
**DM2:** PC English, PC French, PC German/English JewelCase
**Nexus:** Saturn DMDF/DGN format (138 files)
**Theron:** PC Engine CD JP/US Track 02 provenance

## Build

```bash
TMPDIR=/dev/shm cmake -S . -B .codex-scratch/build -G Ninja -DCMAKE_BUILD_TYPE=Debug
TMPDIR=/dev/shm cmake --build .codex-scratch/build -j1
TMPDIR=/dev/shm ctest --test-dir .codex-scratch/build -j2 --output-on-failure
```

Requires SDL3. On macOS: `brew install sdl3`


## Historical statistics (not current metrics)

- **300+ source files**, **245K+ lines of C**
- **365+ headers**, **149+ tests**, **80+ probes**
- CI covers M10 verify, warnings, CMake builds, Phase A, audio probe, and determinism
- **Localization:** M12 launcher has a 19-language UI cycle (`po/startup-menu.<lang>.po`); DM1 in-game strings load via a 19-language candidate list (`po/dm1.<lang>.po`) and fall back to English. `startup-menu` coverage is native for all shipped locales (sv/de/fr/ja/zh/id reviewed earlier; cs/da/es/fi/hu/it/ko/nl/no/pl/pt/ru/tr translated 2026-07-19); `csb` and `theron` domains are fully translated for all shipped locales as of 2026-07-19 (sv/de/fr/ja/zh reviewed earlier; the 13 fallback locales translated 2026-07-19, `validate_po_layout.sh` PASS with 94-100% native coverage). See `po/README.md` + `po/validate_po_layout.sh` for the layout contract.
- **5 games:** DM1, CSB, DM2, DM Nexus, Theron's Quest

These counts and localization claims are historical, unverified snapshots.
Use the current tree, CMake test registration and po/README.md instead.
