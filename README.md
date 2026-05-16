# Firestaff — Dungeon Master Collection

A faithful recreation of the classic Dungeon Master series with modern enhancements.

## Games

| Game | V1 (Original) | V2.1 (Upscaled) | V2.2 (Enhanced) |
|---|---|---|---|
| **Dungeon Master** | ✅ 62 modules | ✅ EPX pipeline | ✅ 20 features |
| **Chaos Strikes Back** | ✅ 8 modules | ✅ Viewport | ✅ 4 features |
| **Dungeon Master II** | ✅ 6 modules | ✅ Indoor+outdoor | ✅ 4 features |
| **DM Nexus** | 🔜 Planned | — | — |

## Features

### V1 — Original Faithful
- 100% ReDMCSB function parity (40/40 source files)
- DM1 PC-34 compatible rendering pipeline
- Original VGA palette, 320×200 resolution
- V1 game tick rate preserved (55ms / 18.2 Hz)

### V2.1 — Upscaled
- EPX/Scale2x pixel art upscaler (no blurry bilinear)
- Palette-aware RGBA pipeline
- 640×400 (2x) or 1280×800 (4x) output
- Original game logic unchanged

### V2.2 — Enhanced
- Smooth movement (interpolated walk/turn/stairs)
- Dynamic lighting (per-tile, torch flicker)
- Particle effects (torch, spell, blood, weather)
- Minimap, journal, message log, damage numbers
- Camera shake, weather FX, achievements
- Auto-save, screenshot, input remap, inventory sort

### Multi-language
- English, Swedish, German, French
- Auto-detects system language (LC_ALL/LANG)
- Swedish: Firestaff original translation (never existed for DM1)
- Language-specific DUNGEON.DAT loading (EN/FR/DE)

## Project Structure

```
src/
  dm1/        DM1 V1 gameplay (62 files)
  dm1v2/      DM1 V2 modules (36 files)
  csb/        CSB V1+V2 (12 files)
  dm2/        DM2 V1+V2 (10 files)
  engine/     Game loop, SDL bridge, assets, input, save (10 files)
  memory/     Memory/cache system (82 files)
  shared/     Common utilities + localization (80 files)
  ui/         Menus, themes, touch (7 files)
  frontend/   Title, entrance, endgame screens (18 files)
tests/        Test suite (149 files, 304 tests)
include/      Headers (310 files)
probes/       Runtime probes (88 files)
tools/        Python verifiers and build tools
docs/         Documentation + release notes
parity-evidence/  ReDMCSB source-lock evidence
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
ctest
```

Requires: CMake 3.20+, C99 compiler, SDL3 (optional, for windowed mode).

## Testing

```bash
cd build && ctest -j$(nproc)
```

304 tests. Known-incomplete tests are marked `WILL_FAIL`.

## Releases

See [GitHub Releases](https://github.com/yeager/firestaff/releases) for
macOS (DMG), Windows (zip/installer), and Linux (DEB/RPM) builds.

## Source References

- **ReDMCSB** (primary): Complete DM1/CSB reverse-engineered C source
- **CSBWin** (secondary): BeipDev's CSB Windows port
- **SKULL.ASM**: DM2 DOS disassembly (522K lines)
- **Greatstone**: DM1 PC-34 dungeon/graphics atlas

## License

See [LICENSE](LICENSE).
