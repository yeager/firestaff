# AGENTS.md — Firestaff Project Guide

## What Is Firestaff?

Source-faithful Dungeon Master engine for modern hardware. Plays and/or is actively bringing up DM1, Chaos Strikes Back (CSB), DM2: Skullkeep, DM Nexus (Saturn), and Theron's Quest with original fidelity, enhanced filters, upscaled assets, or modern visuals.

**Repo:** https://github.com/yeager/firestaff
**License:** MIT
**Language:** C (pure C11, no C++)
**Build:** CMake + SDL3
**Platforms:** macOS (Apple Silicon + Intel), Windows, Linux x86_64, Linux ARM64 / Steam Deck
**Current release:** v2.7.5

## Current Project Status

Firestaff is in active development. DM1 V1 is the strongest playable target today. The other games have source-locked slices, hash-verified launch profiles, and growing runtime coverage, but should not be presented as fully finished unless the current tests and real-asset runtime proof say so.

| Game | Status |
|------|--------|
| DM1 | Playable/source-locked V1 runtime with ongoing visual polish; V2.0/V2.1/V2.2 presentation pipelines exist. |
| CSB | Launch/profile boundary, dungeon model, mechanics, utility/import, rendering slices, and verification gates exist; end-to-end runtime proof is still being hardened. |
| DM2 | Boot/profile, utility, V2 presentation, lighting, HUD, smooth movement, and touch/controller slices exist; V1 dungeon/render/mechanics parity remains active work. |
| Nexus | Saturn DMDF/DGN data, world, render, save/load, mechanics, and V2 presentation slices exist; real-asset handoff proof remains active work. |
| Theron's Quest | V1 parser, rendering, mechanics, progression, save/load, and verification suite exist; positive real-asset launch through Track 02 remains active work. |

Public docs and README text should be honest, user-facing, and sales-friendly: explain what works and what is being hardened, but do not dump debug logs, failed-test counters, pass IDs, or private queue details into public project pages.

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
| `src/theron/` | Theron's Quest: profile, data, rendering, mechanics, progression |
| `src/test/` | Test utilities |
| `include/` | All public headers (~365 files) |
| `tests/` | Integration tests (~149 files) |
| `probes/` | Headless verification probes (Phase A = CI, others = local) |
| `verification-screens/` | Tracked project screenshots suitable for README/public docs |
| `docs/compare/` | Tracked visual comparison assets suitable for public docs |

### Key Files

| File | What It Does |
|------|-------------|
| `src/engine/m11_game_view.c` | Game view: rendering, input dispatch, HUD, dialog overlays (21K LOC) |
| `src/engine/main_loop_m11.c` | SDL event loop, menu↔game transitions, mouse mapping |
| `src/engine/firestaff_game_loop.c` | Asset loading, game tick, V1 command processing |
| `src/ui/menu_startup_m12.c` | Launcher state machine: navigation, game options, launch logic |
| `src/ui/menu_startup_render_modern_m12.c` | Launcher HD renderer (1920×1080 canvas) |
| `src/ui/menu_hit_m12.c` | Launcher mouse hit-testing |
| `src/shared/asset_status_m12.c` | Game version catalog and recursive hash-verified game-data scanner |
| `src/shared/changelog_m12.c` | Version string and changelog text |
| `src/engine/firestaff_accessibility.c` | Accessibility manifest (JSON, atomic writes) |
| `TODO.md` | Open work only; update at least twice daily during active Firestaff work |
| `DONE.md` | Completed/verified work only; update at least twice daily during active Firestaff work |

### Graphics Modes (V1/V2)

| Mode | Resolution | Description |
|------|-----------|-------------|
| V1 Original | 320×200 | Pixel-perfect original |
| V2.0 Filtered | 320×200 + post | CRT scanlines, palette correction |
| V2.1 Upscaled | 3200×2000 | 10× AI upscale |
| V2.2 Modern | 1920×1080 | New 3D-rendered 2D art |

### Asset Discovery and Launch Gating

Firestaff finds game files by **hash**, not filename or path. The configured data root is searched recursively, so users may keep their own folder layout.

ZIP archives and ISO/BIN disc images are valid game-data containers. The scanner should hash entries/files inside those containers and report matches as virtual paths such as `archive.zip::GRAPHICS.DAT` or `disc.iso::DUNGEON.DAT`. ZIP support covers stored entries everywhere and deflated entries when zlib is available at build time. ISO/BIN support is intended for ISO 9660 data images, especially DM2 disc images and the existing Saturn/Nexus path. For DM1/CSB/DM2, archive-backed required files should be materialized into the local Firestaff asset cache before launch so runtime code can keep opening ordinary `GRAPHICS.DAT` / `DUNGEON.DAT` paths.

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
**CSB:** PC 3.4 English, Atari ST 2.0/2.1, Amiga 3.5, Amiga 3.5 Multilanguage
**DM2:** PC English, PC French, PC German/English JewelCase
**Nexus:** Saturn DMDF/DGN format (138 files)
**Theron:** PC Engine CD JP/US Track 02 provenance

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Requires SDL3. On macOS: `brew install sdl3`

### CI Workflows

| Workflow | Trigger | What |
|----------|---------|------|
| `verify.yml` | push to main/develop, PRs | M10 verify, warnings-check, CMake build matrix, Phase A probe, audio probe, cross-platform determinism |
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
- **Greatstone:** http://greatstone.free.fr/dm/g_dm.html (dungeon maps, graphics atlas, 26+ game-version extractions, IMG5 4bpp format, FTL/PAK/Items format specs)
- **DMWeb Encyclopaedia:** http://dmweb.free.fr/ (game-version matrix, per-platform awards & magazine scans, **byte-level file format specs** for GRAPHICS.DAT/animations/data files, FAQ per platform, custom dungeon gallery, clones index)
- See `docs/DMWEB_REFERENCE.md` for the consolidated dmweb + greatstone reference: every page reviewed, what it gives us, and what we still need to fetch.

## Version Management

Version must be synchronized in three places:
1. `CMakeLists.txt` — `project(Firestaff VERSION x.y.z)`
2. `src/ui/menu_startup_m12.c` — `#define FIRESTAFF_VERSION_STRING`
3. `src/shared/changelog_m12.c` — `M12_Changelog_VersionString()`

Release tags: `v2.7.1`, `v2.7.0`, etc. The CMake version should match the latest release.

When changing a release-facing feature, also update `RELEASE_NOTES.md` and make sure README/AGENTS status still matches reality.

## Testing

- **Phase A probe:** `SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe` — headless, no game data needed. Runs in CI.
- **Strict warnings:** CI runs a strict `-Wall -Wextra -Werror` build path.
- **Data scanner smoke:** `./build/firestaff --scan-data` should report found/missing required files without relying on filenames.
- **Integration tests:** `./build/test_*` — individual test binaries. Most need game data.
- **Verification scripts:** `scripts/verify_*.py` — Python scripts that check source-lock invariants.

Before pushing from the main session, run the smallest relevant local verification set, then watch GitHub Actions after push. Subagents may commit but must not push.

## Conventions

- All game-specific code is suffixed `_pc34_compat` (PC 3.4 compatibility layer)
- Headers live in `include/`, never alongside `.c` files
- Comments cite ReDMCSB functions: `/* ReDMCSB: COMMAND.C F0359 line ~120 */`
- Commits reference pass numbers when applicable: `pass602b`, `BUG-007`, etc.
- Subagents commit but NEVER push. Main verifies before push.
- Always push verified main-session changes to GitHub `main`; do not leave verified Firestaff changes only in the local worktree.
- No API keys, tokens, passwords, or secrets in any file. Game data files stay user-supplied.
- Keep `TODO.md` and `DONE.md` current at least twice per day while active Firestaff work is running.
- Public README/release copy should be user-facing and polished. Keep worker logs, debug manifests, queue status, and internal failure counters out of public sales text.

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

### Updating public project docs
1. Keep README honest and sales-friendly.
2. README screenshots must be real in-game screenshots captured from Firestaff or the original games. Do not use generated, illustrated, mocked, branding, or otherwise invented images as README screenshots. Prefer tracked runtime captures from `verification-screens/` or real comparison captures in `docs/compare/`.
3. Include status per game, data-scanner behavior, platforms, graphics modes, build steps, and legal note.
4. Do not include debug data, queue output, pass logs, local-only paths, or private worker status.

## Project Stats

- **300+ source files**, **245K+ lines of C**
- **365+ headers**, **149+ tests**, **80+ probes**
- CI covers M10 verify, warnings, CMake builds, Phase A, audio probe, and determinism
- **Localization:** M12 launcher has a 19-language UI cycle (`po/startup-menu.<lang>.po`); DM1 in-game strings load via a 19-language candidate list (`po/dm1.<lang>.po`) and fall back to English. Translation coverage is currently Swedish-only-completed; other locales ship as `msginit`-generated scaffolds pending translator pass. See `po/README.md` + `po/validate_po_layout.sh` for the layout contract.
- **5 games:** DM1, CSB, DM2, DM Nexus, Theron's Quest
