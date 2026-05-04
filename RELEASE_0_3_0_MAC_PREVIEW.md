# Firestaff v0.3.0 — Mac Preview Release

## What's New

### DM1 V1 Engine (21 modules)
Complete first-pass implementation of the DM1 original engine parity layer:
- **Movement**: full pipeline, command core, timing, collision, movement prediction
- **Viewport**: wall rendering, floor/ceiling/items, creature render, dungeon square structures
- **Game systems**: combat, spell casting, inventory, champion stats/HUD, food/water
- **World**: light/darkness, teleporter/pit chains, stairs/level transitions
- **Audio**: sound system, VBlank timing
- **UI**: text/message display, input command queue, projectile/explosion rendering

All modules source-locked to ReDMCSB (F0267, F0276, F0299, F0303, etc.)

### CSB V1 Foundation
- Game state management with save/load and checksum verification

### DM1 V2 Movement Engine
- Sub-pixel positioning (8.8 fixed-point)
- Smooth interpolation and collision prediction

### Launcher (20 features)
- Animated backgrounds, screenshot gallery, presentation mode
- Touch layout editor, accessibility options, mod browser
- Auto-updater, community hub, benchmark tool

## Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Platform
- macOS (Apple Silicon / arm64)
- Requires SDL3

## Known Limitations
- Preview build — not all modules integrated into main game loop yet
- Original game data files required (not included)
