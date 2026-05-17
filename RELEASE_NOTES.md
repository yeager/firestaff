# Firestaff v1.6.0

Source-faithful Dungeon Master engine with redesigned settings, HiDPI fix, and V2 graphics pipeline.

## What's New

### Settings Menu Redesign
- **5 tabs**: GAME, GRAPHICS, CONTROLS, AUDIO, ACCESSIBILITY
- **Per-game settings**: Graphics Mode, Version, Language, Speed, Movement Mode, Cheats
- **Graphics modes** with descriptive names:
  - **Original** — pixel-perfect 320×200
  - **Original + Filters** — CRT scanlines, palette correction, sharpening (coming soon)
  - **Original 10x Upscale** — AI upscale preserving DM aesthetic (coming soon)
  - **Modern Graphics** — 3D-rendered 2D with modern look (coming soon)
- Unbuilt features clearly **grayed out** in the menu
- All settings available for all 4 games (DM1, CSB, DM2, DM Nexus)

### Control Schemes
- **Original** (default): A/D = turn, Q/E = strafe — faithful to classic Dungeon Master
- **Hybrid**: A/D = strafe, Q/E = turn — modern FPS-style layout
- V1 mode always forces Original scheme
- Selectable in CONTROLS tab

### HiDPI / Retina Fix
- Fixed menu and game appearing zoomed in on macOS Retina and HiDPI displays
- Content now correctly scales to fill the window at native resolution
- Uses `SDL_WINDOW_HIGH_PIXEL_DENSITY` + `SDL_GetWindowSizeInPixels()`

### Graphics Comparison Page
- Interactive web page comparing V1/V2.0/V2.1/V2.2 graphics tiers
- Drag slider for side-by-side comparison
- Feature comparison table
- Live at https://yeager.github.io/firestaff/compare/

### Code Review & Bug Fixes
- Full review of all 307 source files
- **BUG-007**: Spell mana cost bounds check (ReDMCSB SYMBOL.C F0399)
- **BUG-008**: Column-major tile indexing unified (ReDMCSB DUNGEON.C F0151)
- **BUG-010**: Integer overflow in game ID generation (ReDMCSB LOADSAVE.C F0435)
- 298/304 tests passing (98%)

### V2 Graphics Pipeline (Preview)
- Asset extraction tool: 543/713 DM1 sprites extracted from GRAPHICS.DAT
- V2 animation smoothing planned for V2.1/V2.2 (smooth movement, turning, interpolation)
- GPT-5.5 10x upscale batch starting Wednesday

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
