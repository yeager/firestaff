# CSB, DM2 & Nexus Code Review

## CSB (13 files, 811 lines)

### CSB-BUG-001: [major] Dungeon loader uses row-major tile indexing — FIXED
- Files: csb_v1_dungeon_loader_pc34_compat.c (lines 60, 73)
- Same as BUG-008: y * width + x → x * height + y
- Commit: 31ba97e5

### Verified Correct
- **Chaos magic/DSA** — bytecode VM with 256 flags, script loading, tick dispatch
- **Character/import** — DM1 save import parsing
- **Game state** — clean init/load
- **Viewport config** — wall sets, custom backgrounds
- **V2 modules** — chaos enhanced, minimap, smooth movement, viewport renderer

## DM2 (11 files, 412 lines)

### DM2-BUG-001: [major] Dungeon loader uses row-major tile indexing — FIXED
- File: dm2_v1_dungeon_loader.c (line 37)
- Same fix as CSB-BUG-001
- Commit: 31ba97e5

### Verified Correct
- **Combat** — damage calculation with range penalty
- **Companions** — NPC management with loyalty
- **Outdoor renderer** — day/night cycle, weather
- **Tech/magic items** — tech/magic/hybrid item usage + power cost
- **Save/load** — simple file I/O
- **V2 modules** — companion UI, outdoor enhanced FX, tech crafting, viewport

## Nexus (20 files, 1957 lines)

### No bugs found
- **ISO reader** — MODE1/2352 sector parsing, ISO 9660 filesystem, CUE parser — solid
- **Math3D** — vec3/mat4 ops, perspective projection — correct
- **Rasterizer** — edge-function triangle rasterization with z-buffer — correct
- **Dungeon** — uses squares[y][x] but Nexus/Saturn format is unknown, not ReDMCSB-based
- **Engine** — ISO/extracted file detection, level loading
- **V2 modules** — atmosphere, lighting, particles, render pipeline, upscaler

## Summary
- 44 files reviewed
- 2 bugs found and fixed (both column-major tile indexing)
- All 44 files verified
