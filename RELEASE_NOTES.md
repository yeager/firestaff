# Firestaff v1.5.0

Source-faithful Dungeon Master engine — code review release with critical bugfixes, ARM64 Linux (Steam Deck) support, and V2 graphics pipeline.

## What's New Since v1.4.0

### Bug Fixes
- **BUG-007**: Spell mana cost bounds check on power symbol index (ReDMCSB SYMBOL.C F0399)
- **BUG-008**: Column-major tile indexing documented and unified with `dm1_tile_index()` helper (ReDMCSB DUNGEON.C F0151)
- **BUG-010**: Integer overflow in game ID generation fixed (ReDMCSB LOADSAVE.C F0435)

### New Features
- **ARM64 Linux builds** — native DEB + RPM for Steam Deck
- **DM2 dungeon hash** — hash-based asset discovery for Dungeon Master II
- **Graphics comparison page** — interactive V1/V2.0/V2.1/V2.2 showcase at https://yeager.github.io/firestaff/compare/
- **Asset extraction tool** — `extract_all_sprites` dumps all GRAPHICS.DAT sprites as PNG with manifest

### V2 Graphics Pipeline (Preview)
- V2.0: Original + filter pipeline (CRT, palette correction, sharpening)
- V2.1: 10× AI upscale (GPT-5.5, coming soon)
- V2.2: Modern 3D-rendered 2D art (coming soon)
- Smooth animation interpolation for V2.1/V2.2

### Code Quality
- Full code review of all 307 source files
- 297/304 tests passing (97.7%)
- Column-major collision regression test added
- Stale WILL_FAIL gates cleaned up

## Platforms

| Platform | Architecture | Format |
|----------|-------------|--------|
| macOS | arm64 | DMG, ZIP |
| Windows | x86_64 | ZIP, Installer (EXE) |
| Linux | x86_64 | DEB, RPM |
| Linux | ARM64 (Steam Deck) | DEB, RPM |

## Requirements

- Original game data files (DUNGEON.DAT, GRAPHICS.DAT) — not included
- SDL3 runtime library
