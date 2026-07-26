# Firestaff - Project Guide

## What is Firestaff?

Firestaff is a C application that reimplements the Dungeon Master (DM1) and Chaos Strikes Back (CSB) game engines with source-level parity to the original PC 3.4 release. It uses **ReDMCSB** (a reconstructed C source of the original) as the authoritative reference. The project also supports DM2 (Dungeon Master II) via a separate runtime path.

The codebase targets macOS, Linux, and Windows. It renders via SDL3 and uses no external game-engine frameworks.

## Build

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=cc
cmake --build build
```

- **Compiler**: use `cc` (system clang on macOS). Do not use gcc.
- **Language**: pure C (no C++). The project enforces `LANGUAGES C` in CMakeLists.txt.
- **Version**: set in `CMakeLists.txt` line ~3505 as `project(Firestaff VERSION x.y.z LANGUAGES C)`.

## Test

```bash
ctest --test-dir build -j4
```

There are ~3200 tests. Some viewport and boot tests require original game data files and will fail/timeout without them. Two pre-existing test failures are known:
- `test_theron_v1_startup_save_resume_pc34.c:332` syntax error
- `dm1_v1_viewport_door_wall_ornament_source_lock` missing function

Run a subset with `-R <pattern>`:
```bash
ctest --test-dir build -R "viewport" -j4 --output-on-failure
```

## Release

- Bump version in `CMakeLists.txt`, commit, tag with `v3.0.XXX`, push with `--tags`.
- GitHub Actions release workflow triggers on `v*` tags and creates a GitHub Release.
- CI runs on Ubuntu 24.04, macOS 14, and Windows 2022 (verify.yml).

## Pre-commit hooks (lefthook)

Three checks run on every commit:
- `newline_eof`: all committed files must end with a newline
- `trailing_whitespace`: no trailing whitespace
- `no_game_data_payloads`: no original game data files may be tracked by git

## Project structure

```
src/
  engine/     - M11 game view (main runtime: ~50k lines in m11_game_view.c)
  csb/        - CSB-specific modules (viewport, boot, runtime, DSA)
  dm1/        - DM1-specific modules (viewport 3D, music, movement, etc.)
  dm1v2/      - DM1 V2 presentation layer (camera, modern rendering)
  dm2/        - DM2 runtime
  shared/     - Cross-game: audio (SDL3), rendering, main entry
  memory/     - Dungeon data decoding, save/load, combat serialization
  frontend/   - UI frontend, dialog, text rendering
  audio/      - Audio decoding (SND, SONG.DAT)
include/      - All public headers (~1980 files)
tests/        - Test sources (~2960 files)
parity-evidence/ - Source-lock evidence documents (pass NNN)
```

## Key architecture

### Two viewport rendering paths

1. **M11 DM1 path** (`m11_draw_viewport` in `m11_game_view.c`): comprehensive DM1 viewport renderer with inline dungeon data access, wall ornament resolution, creature/item drawing. This is the live game path.

2. **CSB path** (`csb_v1_viewport_render_frame` → `dm1_viewport_3d_draw_frame`): CSB-specific viewport using callback-based architecture. Wall ornament ordinals come via `DM1_ViewportWallOrnamentOrdinalCallback`. Element routing classifies grid cells into wall/corridor/pit/stairs/door/teleporter.

### Game state

- `M11_GameViewState` (defined in `include/m11_game_view.h`): monolithic state struct (~1570 lines). Contains world state, party, dungeon, audio, viewport, HUD, DM2, and presentation state.
- Dungeon data: `memory_dungeon_dat_pc34_compat.h` defines `DungeonThings_Compat` (thing lists), `DungeonMapDesc_Compat` (map metadata), and thing type macros.

### Music (F0740-F0743)

- `dm1_v1_f0740_f0743_music_source_pc34_compat.h`: DM1 music state machine model
- Wired into M11 via `dm1MusicSource`/`dm1MusicState`/`dm1MusicDriver` fields
- SONG.DAT bound at init; F0742 map track on stairs/teleporter; F0743 update per tick
- Only game-won track (C2) is proven playable; other tracks remain fail-closed

### Wall ornament ordinals

- Random ornaments: `dm1_v1_random_ornament_pc34_compat.h` (F0169/F0170/F0171)
- Sensor ornaments: walk thing list, check `sensor.ornamentOrdinal`
- Provider module: `dm1_v1_viewport_wall_ornament_ordinal_provider_pc34_compat.h` bridges both into the viewport callback

## Work priorities

From TODO.md, the active queue is Q-DM1-01 through Q-DM1-10 and Q-CSB-01 through Q-CSB-05. Priority order:

1. **Q-DM1-08** Startup audio and cadence (SWSH, title, entrance, music)
2. **Q-DM1-03** Dungeon viewport material matrix (F0107-F0115 routing)
3. **Q-DM1-07** Action and spell HUD (C010/C011, cursor, cooldown)
4. Combat, inventory, door/sensor topology come after

**Startup, HUD, and viewport are prioritized before the combat system.**

## Workflow

- `TODO.md`: open work items with progress notes and version tags
- `DONE.md`: completed items (newest first)
- `RELEASE_NOTES.md`: per-version changelog
- After each successful job: bump version, update TODO/DONE/RELEASE_NOTES, commit, tag, push
- Focus on DM1/CSB; use ReDMCSB source code as the reference
- Merge smaller jobs into larger releases; do the biggest and most important features first
- Push to GitHub main after each successful job

## Naming conventions

- Headers: `dm1_v1_{feature}_pc34_compat.h` (DM1 PC 3.4 compatibility)
- Sources: `dm1_v1_{feature}_pc34_compat.c` in `src/dm1/`
- CSB modules: `csb_v1_{feature}_pc34_compat.{h,c}`
- Functions: `dm1_v1_{feature}_{function}_pc34()` or `F0NNN_UPPERCASE_Name_Compat()`
- Test files: `test_{module_name}.c` in `tests/`
- Parity evidence: `pass{NNN}_{description}.md` in `parity-evidence/`

## Source references

ReDMCSB function identifiers (F0107, F0740, etc.) map to the reconstructed C source. Comments reference these as `ReDMCSB FILENAME.C:LINE` or `F0NNN_FUNCTION_NAME`. The `pc34` suffix means PC version 3.4 (the canonical DOS release).

## Things to avoid

- Do not use gcc; use `cc` (system clang)
- Do not commit original game data files (DUNGEON.DAT, GRAPHICS.DAT, etc.)
- Do not synthesize audio or graphics from non-original sources; only bind verified original media
- Do not add C++ code; the project is pure C
- Do not skip pre-commit hooks (--no-verify) without good reason
