# 🔥 Firestaff

**Source-faithful Dungeon Master engine for modern hardware.**

Play all four Dungeon Master games — DM1, Chaos Strikes Back, Dungeon Master II, and DM Nexus — with original fidelity, enhanced filters, AI-upscaled graphics, or fully modern visuals.

[![Release](https://img.shields.io/github/v/release/yeager/firestaff)](https://github.com/yeager/firestaff/releases/latest)
[![Tests](https://img.shields.io/badge/tests-297%2F304-brightgreen)](https://github.com/yeager/firestaff/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Steam%20Deck-orange)]()

## Games

| Game | Status | Source Reference |
|------|--------|-----------------|
| Dungeon Master (DM1) | ✅ Playable | ReDMCSB (primary) |
| Chaos Strikes Back | ✅ Playable | ReDMCSB + CSBWin |
| Dungeon Master II: Skullkeep | 🔧 In progress | — |
| DM Nexus (Saturn) | 🔧 In progress | — |

## Graphics Modes

| Mode | Description |
|------|-------------|
| **V1 Original** | Pixel-perfect 320×200, exactly as the original |
| **V2.0 Filtered** | Original graphics + CRT scanlines, palette correction, sharpening |
| **V2.1 Upscaled** | 10× AI upscale preserving the DM aesthetic |
| **V2.2 Modern** | Entirely new 3D-rendered 2D art with a modern look |

Switch between modes at runtime — no restart needed.

## Features

- **Source-locked**: Every system cross-referenced against [ReDMCSB](http://dmweb.free.fr/) source code
- **307 source files** with full code review, **297/304 tests passing**
- **Cross-platform**: macOS (Apple Silicon + Intel), Windows, Linux x86_64, Linux ARM64 (Steam Deck)
- **Touchscreen & gamepad**: Full touch/click zones, controller support
- **20 languages**: EN, SV, DE, FR, ES, IT, PT, NL, PL, CS, RU, JA, KO, ZH, DA, NO, FI, HU, TR + auto-detection
- **Save system**: FSSV format with CRC32 integrity, 10 slots per game
- **Data validation**: SHA256 hash verification for all game files
- **Asset discovery**: Hash-based — place your files anywhere, Firestaff finds them

## Download

**[Latest release →](https://github.com/yeager/firestaff/releases/latest)**

| Platform | Formats |
|----------|---------|
| macOS | DMG, ZIP |
| Windows | Installer (EXE), ZIP |
| Linux x86_64 | DEB, RPM |
| Linux ARM64 (Steam Deck) | DEB, RPM |

## Quick Start

1. Download and install Firestaff for your platform
2. Run Firestaff — it creates `~/.firestaff/data/` automatically
3. Place your original game files:
   ```
   ~/.firestaff/data/
     DUNGEON.DAT        ← DM1 (auto-detected by hash)
     GRAPHICS.DAT       ← DM1
     csb/DUNGEON.DAT    ← Chaos Strikes Back
     csb/GRAPHICS.DAT
     dm2/DUNGEON.DAT    ← Dungeon Master II
     dm2/GRAPHICS.DAT
     nexus/             ← DM Nexus files
   ```
4. Play!

> **Windows**: Files go in `%APPDATA%\Firestaff\data\`

## Building from Source

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

### Requirements
- CMake 3.20+
- C11 compiler (GCC, Clang, MSVC)
- SDL3

### Run Tests
```bash
cd build && ctest --output-on-failure
```

## Architecture

```
src/
  engine/    — Core engine: game loop, rendering, input, save, audio
  dm1/       — DM1 V1 source-locked engine (movement, combat, spells, creatures)
  dm1v2/     — DM1 V2 enhanced features (weather, viewport, settings)
  csb/       — Chaos Strikes Back engine
  dm2/       — Dungeon Master II engine
  nexus/     — DM Nexus (Saturn format support)
  memory/    — Graphics cache, memory management, savegame serialization
  ui/        — Menu system, bestiary, spell reference, map viewer
  shared/    — Asset loading, audio, fonts, localization, data validation
  frontend/  — Title screen, entrance, endgame, boot sequence
include/     — All headers (364 files)
tests/       — 304 automated tests
tools/       — Asset extraction, verification gates, probe tools
parity-evidence/ — Source-lock manifests and verification reports
```

## Source Fidelity

Every game system is cross-referenced against the original source code:

- **Movement**: `MOVESENS.C` F0267 collision, F0268 step, turning, stairs
- **Combat**: `CHAMPION.C` F0304 experience, `COMBAT.C` attack/defense
- **Spells**: `SYMBOL.C` F0399 encoding, mana cost multipliers
- **Creatures**: `GROUPMAN.C` AI behavior, movement timing
- **Viewport**: `DUNVIEW.C` F0128 wall composition, depth rendering
- **Save/Load**: `LOADSAVE.C` F0433/F0435 serialization with CRC32
- **Memory**: Cache allocator, defrag, graphics block management

Verification gates ensure no regression against source evidence.

## Localization

All strings use gettext PO files. Add a new language:
1. Copy `po/firestaff.pot` to `po/firestaff.xx.po`
2. Translate msgstr entries
3. Firestaff auto-detects system language

## Legal

Firestaff is a clean-room engine reimplementation based on publicly available source code references. It requires original game data files that you legally own. **No copyrighted game data is included.**

Dungeon Master, Chaos Strikes Back, and Dungeon Master II are trademarks of FTL Games.
DM Nexus is a trademark of Victor Interactive Software.

## License

MIT

## Credits & References

- **[ReDMCSB](http://dmweb.free.fr/)** — Primary reference implementation (WIP 2021-02-06)
- **[CSBWin](https://github.com/BeipDev/CSBWin)** — CSB reference source
- **[Greatstone](http://greatstone.free.fr/dm/)** — DM data format documentation and atlas
- **[DMWeb](http://dmweb.free.fr/)** — Dungeon Master community and resources
