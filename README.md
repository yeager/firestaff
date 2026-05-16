
# 🔥 Firestaff

**Open-source Dungeon Master engine reimplementation.**

Play all four Dungeon Master games — DM1, Chaos Strikes Back, Dungeon Master II, and DM Nexus — with original fidelity or enhanced visuals, on modern hardware.

## Games

| Game | V1 Original | V2.1 Upscaled | V2.2 Enhanced |
|------|:-----------:|:-------------:|:-------------:|
| Dungeon Master | ✅ | ✅ EPX 2× | ✅ 20 features |
| Chaos Strikes Back | ✅ | ✅ | ✅ |
| Dungeon Master II | ✅ | ✅ | ✅ |
| DM Nexus (Saturn) | ✅ ISO reader | ✅ EPX 2× | ✅ Lighting + particles |

## Features

- **Original fidelity (V1)**: Pixel-perfect recreation at 320×200
- **Upscaled (V2.1)**: EPX/Scale2x pixel-art scaling to 640×400+
- **Enhanced (V2.2)**: Dynamic lighting, particles, fog, ambient occlusion, minimap, journal, achievements
- **20 languages**: EN, SV, DE, FR, ES, IT, PT, NL, PL, CS, RU, JA, KO, ZH, DA, NO, FI, HU, TR + system detection
- **Cross-platform**: macOS (Apple Silicon + Intel), Windows, Linux
- **Touchscreen**: Full touch/click support
- **Controller**: Gamepad support (Xbox, PlayStation, Nintendo)
- **Saturn ISO**: DM Nexus reads directly from CUE/BIN disc images
- **Data validation**: SHA256 hash verification for all 148 game files
- **Auto-setup**: Creates data directories on first run
- **Save system**: FSSV format, 10 slots per game, cross-game

## Quick Start

1. Download the latest release for your platform
2. Run Firestaff — it creates `~/.firestaff/data/` automatically
3. Place your original game files:
   ```
   ~/.firestaff/data/
     dm1/GRAPHICS.DAT + DUNGEON.DAT
     csb/GRAPHICS.DAT + DUNGEON.DAT
     dm2/GRAPHICS.DAT + DUNGEON.DAT
     nexus/YourDisc.cue + .bin files
   ```
4. Run `firestaff --validate` to verify your files
5. Play!

### Windows
Files go in `%APPDATA%\Firestaff\data\`

## Building from Source

### Requirements
- CMake 3.16+
- C11 compiler (GCC, Clang, MSVC)
- SDL2 (optional, for windowed mode)
- SDL2_mixer (optional, for CD audio)

### Build
```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Run Tests
```bash
cd build
ctest --output-on-failure
```

## Architecture

```
src/
  engine/    — Core engine: game loop, assets, input, save, audio
  dm1/       — DM1 V1 original engine
  dm1v2/     — DM1 V2 enhanced features
  csb/       — Chaos Strikes Back
  dm2/       — Dungeon Master II
  nexus/     — DM Nexus (Saturn ISO, 3D renderer, DMDF models)
  ui/        — Menu, inventory, spell UI, HUD
  shared/    — PO loader, utilities
  frontend/  — Title screen, startup
include/     — All headers
po/          — Localization (gettext PO files)
tests/       — 304 automated tests
docs/        — Setup guide, hash list, architecture
assets/      — Icons, .desktop, Info.plist
```

## Localization

All strings use gettext PO files. Add a new language:
1. Copy `po/firestaff.pot` to `po/firestaff.xx.po`
2. Translate msgstr entries
3. Firestaff auto-detects system language

## Legal

Firestaff is a clean-room engine reimplementation. It requires original game data files that you legally own. No copyrighted game data is included in this repository.

Dungeon Master, Chaos Strikes Back, and Dungeon Master II are trademarks of FTL Games. DM Nexus is a trademark of Victor Interactive Software.

## License

MIT

## Credits

- [ReDMCSB](http://dmweb.free.fr/) — Primary reference implementation
- [CSBWin](https://github.com/BeipDev/CSBWin) — CSB reference source
- [Greatstone](http://greatstone.free.fr/dm/) — DM data format documentation

