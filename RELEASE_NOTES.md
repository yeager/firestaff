# Firestaff v0.4.0

Source-faithful Dungeon Master engine — now with multi-game support and critical bugfixes.

## What's New

### Bug Fixes
- **BUG-007**: Spell mana cost now validates power symbol index bounds, preventing array over-read if symbol encoding is unexpected (per ReDMCSB SYMBOL.C F0399)
- **BUG-008**: Documented and unified column-major tile indexing across collision and dungeon loader, with `dm1_tile_index()` helper (per ReDMCSB DUNGEON.C F0151)
- **BUG-010**: Fixed integer overflow in game ID generation — `rand() * 65536` overflows uint32; now uses proper shift-and-mask (per ReDMCSB LOADSAVE.C F0435)

### DM2 Support
- Added verified DM2 dungeon hash for hash-based asset discovery
- DM2 game data now included in asset pipeline

### Test Improvements
- Added asymmetric column-major collision regression test (catches row/column indexing bugs that symmetric grids miss)
- Removed stale `WILL_FAIL` on pass499 wall occlusion evidence gate (now unlocked)
- Fixed CMakeLists trailing whitespace that broke `git diff --check` gate
- **297/304 tests pass** (97.7%) — remaining are SDL display probes (3) and intentional CSB launch blockers (2)

### Code Review
- Full code review of all 307 source files across DM1 V1/V2, CSB, DM2, Nexus, shared, and memory subsystems
- All parity evidence manifests updated from clean test run

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | Universal (arm64 + x86_64) | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 (Steam Deck) | DEB, RPM |

## Requirements

- Original Dungeon Master game data files (DUNGEON.DAT, GRAPHICS.DAT) — not included
- SDL3 runtime library

## Notes

This is a preview release for the ongoing source-backed Dungeon Master compatibility project. Game data from the original Dungeon Master, Chaos Strikes Back, and Dungeon Master II is required but not bundled.
