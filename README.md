# 🔥 Firestaff

**Source-faithful Dungeon Master engine for modern hardware.**

Play all five Dungeon Master games — DM1, Chaos Strikes Back, Dungeon Master II, DM Nexus, and Theron's Quest — with original fidelity, enhanced filters, AI-upscaled graphics, or fully modern visuals.

[![Release](https://img.shields.io/github/v/release/yeager/firestaff)](https://github.com/yeager/firestaff/releases/latest)
[![Tests](https://img.shields.io/badge/tests-361%2F392-brightgreen)](https://github.com/yeager/firestaff/actions)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Steam%20Deck-orange)]()

## Games

| Game | Status | Phases | Source Reference |
|------|--------|--------|-----------------|
| Dungeon Master (DM1) | ✅ Playable | Complete | ReDMCSB (primary) |
| Chaos Strikes Back | ✅ Playable | Complete | ReDMCSB + CSBWin |
| Dungeon Master II: Skullkeep | 🔧 In progress | Stable | skproject |
| DM Nexus (Saturn) | 🔧 In progress | 2 of 8 | Saturn DMDF/DGN |
| Theron's Quest | 🔧 In progress | 2 of 8 | ReDMCSB (subset) |

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
- **307 source files**, **365 headers**, **245K+ lines of C** — all code-reviewed
- **361/392 tests passing** — 31 integration gates actively in development
- **Cross-platform**: macOS (Apple Silicon + Intel), Windows, Linux x86_64, Linux ARM64 (Steam Deck)
- **Touchscreen & gamepad**: Full touch/click zones, controller support
- **20 languages**: EN, SV, DE, FR, ES, IT, PT, NL, PL, CS, RU, JA, KO, ZH, DA, NO, FI, HU, TR + auto-detection
- **Save system**: FSSV format with CRC32 integrity, 10 slots per game
- **Data validation**: SHA256 hash verification for all game files
- **Asset discovery**: Hash-based — place your files anywhere, Firestaff finds them
- **95 headless probes**: Live-capture verification gates for CI and local regression testing

## Download

**[Latest release →](https://github.com/yeager/firestaff/releases/latest)**
— macOS DMG · Windows EXE/ZIP · Linux DEB/RPM (x86_64 + ARM64)

| Platform | Download |
|----------|----------|
| macOS (Apple Silicon / Intel) | [Firestaff-macos.dmg](https://github.com/yeager/firestaff/releases/latest) |
| Windows | [Firestaff-windows.exe](https://github.com/yeager/firestaff/releases/latest) · [ZIP](https://github.com/yeager/firestaff/releases/latest) |
| Linux x86_64 | [DEB](https://github.com/yeager/firestaff/releases/latest) · [RPM](https://github.com/yeager/firestaff/releases/latest) |
| Linux ARM64 (Steam Deck) | [DEB](https://github.com/yeager/firestaff/releases/latest) · [RPM](https://github.com/yeager/firestaff/releases/latest) |

All releases include SHA256 checksums. See [RELEASE_NOTES.md](./RELEASE_NOTES.md) for what's new.

## Quick Start

1. **Download and install** from [Releases](https://github.com/yeager/firestaff/releases/latest)
2. **Run Firestaff once** — it creates `~/.firestaff/data/` automatically
3. **Place your original game files** (auto-detected by SHA256 hash):

   **Dungeon Master (DM1)**
   ```
   ~/.firestaff/data/
     DUNGEON.DAT   ← DM1 dungeon data
     GRAPHICS.DAT  ← DM1 graphics
   ```

   **Chaos Strikes Back**
   ```
   ~/.firestaff/data/csb/
     DUNGEON.DAT   ← CSB dungeon data
     GRAPHICS.DAT  ← CSB graphics
   ```

   **Dungeon Master II**
   ```
   ~/.firestaff/data/dm2/
     DUNGEON.DAT   ← DM2 dungeon data
     GRAPHICS.DAT  ← DM2 graphics
   ```

   **DM Nexus** (Saturn format)
   ```
   ~/.firestaff/data/nexus/
     (all Nexus data files)
   ```

   **Theron's Quest** (PC Engine CD)
   ```
   ~/.firestaff/data/theron/
     (all Theron's Quest data files — JP and US releases supported)
   ```

4. **Play!** Firestaff auto-detects which game you have.

> **Windows**: `%APPDATA%\Firestaff\data\`
> **macOS**: `~/Library/Application Support/Firestaff/data/`

### Command-Line Options

```
firestaff [options]
  --duration <ms>    Run for specified milliseconds (-1 = run until exit, 0 = single frame)
  --width <px>        Window width (default: 640)
  --height <px>       Window height (default: 400)
  --scale-mode <n>    Graphics mode: 1=V1 (original), 2=V2.1 (enhanced), 3=V2.2 (modern)
  --script <cmds>     Comma-separated input script: up,down,left,right,enter,esc
  --data-dir <path>   Asset directory (default: FIRESTAFF_DATA env var or system location)
  --game <id>         Pre-select game: dm1, csb, dm2, nexus1, theron
  --fullscreen        Run in fullscreen mode
  --no-vsync          Disable vertical sync
  --fps               Show FPS counter
  --version           Show version and exit
  --help, -h          Show this help
```

**Game detection:** Firestaff auto-detects which game you have from the `data/` directory.
Game order: Dungeon Master → Chaos Strikes Back → Dungeon Master II → Dungeon Master Nexus → Theron's Quest.

**Scale modes:** V1 (original 320×200 upscaled), V2.1 (640×400), V2.2 (960×600 with smooth scaling).

Examples:
```bash
firestaff --duration 5000       # run for 5 seconds then exit
firestaff --scale-mode 2        # V2.1 enhanced graphics
firestaff --data-dir ~/my/dm1   # custom data directory
firestaff --fullscreen --fps    # fullscreen with FPS display
firestaff --game theron         # launch Theron's Quest directly
```

## Building from Source

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/firestaff --help
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
  engine/    — Core engine: game loop, rendering, input, save, audio (M11/M12)
  dm1/       — DM1 V1 source-locked engine (movement, viewport, sensors)
  dm1v2/     — DM1 V2 enhanced features (viewport, weather, settings)
  csb/       — Chaos Strikes Back engine
  dm2/       — Dungeon Master II engine
  nexus/     — DM Nexus (Saturn format support)
  theron/    — Theron's Quest engine (subset of DM1, PC Engine CD)
  shared/    — Asset loading, audio, fonts, localization, data validation
  ui/        — Menu system, bestiary, spell reference, map viewer
  frontend/  — Title screen, entrance, endgame, boot sequence
  memory/    — Dungeon, graphics, movement, combat, timeline, savegame
probes/      — 95 live-capture headless probes for CI and regression
include/     — All 365 headers
tests/       — Integration tests
parity-evidence/ — Source-lock manifests per feature/game
```

## Source Fidelity

Every game system is cross-referenced against the original source code:

- **ReDMCSB** ([dmweb.free.fr](http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z)) — Primary DM1 reference: `MOVESENS.C`, `CHAMPION.C`, `PANEL.C`, `DUNVIEW.C`, `COMBAT.C`, `LOADSAVE.C`
- **skproject** ([github.com/gbsphenx/skproject](https://github.com/gbsphenx/skproject)) — DM2 source reference
- **CSBWin** ([github.com/BeipDev/CSBWin](https://github.com/BeipDev/CSBWin)) — CSB reference source
- **CSB** ([github.com/zelurker/CSB](https://github.com/zelurker/CSB)) — CSB lineage reference
- **Greatstone** ([greatstone.free.fr](http://greatstone.free.fr/dm/)) — Dungeon map and graphics atlas

Key reference files:
- `DUNVIEW.C` — Viewport wall rendering, depth draw order (F0128)
- `MOVESENS.C` — Movement, collision, turn, step (F0267–F0268)
- `PANEL.C` — Champion HUD, food/water/poison labels, stat panel
- `CHAMPION.C` — Combat, experience, equipment slots (F0300–F0302)
- `TIMELINE.C` / `CLIKVIEW.C` — Floor sensors and wall interactions
- `LOADSAVE.C` — Save/load serialization with CRC32 (F0433–F0435)

Verification gates ensure no regression against source evidence.

## Theron's Quest — Development Status

Theron's Quest (1990, Working Designs) is a "light" version of DM1 for the PC Engine CD. It uses a subset of DM1 items, creatures, and spells across 7 mini-dungeons. Development is at Phase 2 of 8:

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 0 — Provenance | ✅ Complete | Extracted file set (138 files), MD5/SHA256 verified |
| Phase 1 — Runtime Profile | ✅ Complete | Separate boot/runtime, asset roots, save namespace, diagnostics |
| Phase 2 — Data Formats | ✅ Complete | All dungeon, object, text, champion, creature, graphics formats |
| Phase 3 — Core World Model | ❌ Pending | Map loading, party placement, transitions, timers |
| Phase 4 — Rendering Pipeline | ❌ Pending | Wall/floor/object/creature rendering, palette, PC Engine planar fallback |
| Phase 5 — Gameplay Systems | ❌ Pending | Combat, magic, puzzles, dungeon logic |
| Phase 6 — Audio | ❌ Pending | Speech, music, SFX (CD audio) |
| Phase 7 — UI/UX | ❌ Pending | Menus, champion management, save/load |
| Phase 8 — Verification Suite | ❌ Pending | Asset manifests, parser fixtures, deterministic input scripts |

Both JP (MD5: b7afb338ad31be1025b53f9aff12d73a) and US (MD5: f23601102138f87c33025877767ebf76) releases are supported.

## Localization

All strings use gettext PO files. Add a new language:
1. Copy `po/firestaff.pot` to `po/firestaff.xx.po`
2. Translate msgstr entries
3. Firestaff auto-detects system language

## Legal

Firestaff is a clean-room engine reimplementation based on publicly available source code references. It requires original game data files that you legally own. **No copyrighted game data is included.**

Dungeon Master, Chaos Strikes Back, and Dungeon Master II are trademarks of FTL Games.
DM Nexus is a trademark of Victor Interactive Software.
Theron's Quest is a trademark of Working Designs / Victor Interactive Software.

## License

MIT

## Credits & References

- **[ReDMCSB](http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z)** — Primary DM1 source reference (WIP 2021-02-06)
- **[skproject](https://github.com/gbsphenx/skproject)** — DM2/Skullkeep source reference
- **[CSBWin](https://github.com/BeipDev/CSBWin)** — Chaos Strikes Back reference source
- **[CSB](https://github.com/zelurker/CSB)** — CSB lineage reference
- **[Greatstone](http://greatstone.free.fr/dm/)** — DM data format documentation and atlas
- **[DMWeb](http://dmweb.free.fr/)** — Dungeon Master community and resources
- **[Theron's Quest](http://dmweb.free.fr/?q=node/1585)** — Additional DM1 quest pack reference