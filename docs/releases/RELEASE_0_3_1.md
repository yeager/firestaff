# Firestaff v0.3.1

**Release date:** 2026-05-04

## Highlights

This release delivers a massive DM1 V1 parity push — 20+ source-locked systems directly from the ReDMCSB reference source, plus 20 new launcher features and full repo reorganization.

## DM1 V1 — Source-locked systems

All implementations are traced back to specific ReDMCSB source files and function references:

- **Endgame system** — Firestaff assembly, Fuse action, Lord Chaos confrontation (ENDGAME.C)
- **Resurrection & reincarnation** — champion revival mechanics (CHAMPION.C)
- **Skill/experience progression** — XP gain, skill advancement (CHAMPION.C)
- **Message/text display** — scrolling message area, string rendering (TEXT.C, DRAWMSGA.C)
- **Sensor/trigger system** — pressure plates, wall switches, effect dispatch (MOVESENS.C)
- **Light & torch system** — dynamic dungeon lighting (CHAMPION.C, DEFS.H)
- **Creature AI behavior** — pathfinding, aggression, group tactics (GROUP.C)
- **Object/item interaction** — pickup, drop, throw, use (WORLD.C)
- **Event timer & scheduler** — timeline events, delayed actions (TIMELINE.C, GAMELOOP.C)
- **Champion needs** — food, water, rest mechanics (CHAMPION.C)
- **Save/load serialization** — game state persistence
- **Combat & damage** — attack resolution, defense calculations (GROUP.C, CHAMPION.C)
- **Sound/music** — audio event dispatch (SOUND.C, MUSIC.C)
- **Spell casting** — rune combinations, spell effects
- **Inventory management** — container logic, weight, slot rules
- **Champion stats** — attribute system, stat display

## Launcher features (20 new)

- Audio settings, changelog viewer, data file validator
- Accessibility options, theme selector, quick resume
- Save game browser, input remapping, custom dungeon importer
- Bestiary, item encyclopedia, map viewer, spell reference
- Music jukebox, gamepad config, campaign mode
- Touch layout editor, animated background, screenshot gallery
- Presentation mode preview

## Repo organization

- Moved 56 documentation files and 3 JSON data files from repo root into organized subdirectories:
  - `docs/plans/`, `docs/releases/`, `docs/parity/`, `docs/audio/`, `docs/graphics/`, `docs/design/`, `docs/v2/`
  - `data/` for JSON files
- Root now contains only `README.md`, `CONTRIBUTING.md`, `SECURITY.md`

## Bug fixes

- Fixed missing `#include <stdint.h>` for Windows builds

## Build

- macOS ARM64 (Apple Silicon)
- Requires SDL3
- CMake build system
