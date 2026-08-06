# Firestaff

**Five Dungeon Master games. One engine. Your original data.**

Firestaff is a source-faithful reimplementation of the major Dungeon Master
game engines, built from scratch in portable C for macOS, Windows, Linux,
Steam Deck, iOS and Android. It uses your legally owned game files on modern
hardware with pixel-perfect **Original** rendering or selectable **Custom**
presentation at resolutions from 640×400 up to 4K. DM1 is the strongest
playable target; the other games are brought up behind explicit source-lock
and real-data verification gates.

[![Release](https://img.shields.io/github/v/release/yeager/firestaff)](https://github.com/yeager/firestaff/releases/latest)
[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Steam%20Deck%20%7C%20iOS%20%7C%20Android-orange)]()

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## Supported Games

| Game | Platform | Reference Source | Status |
|------|----------|-----------------|--------|
| **Dungeon Master** | DOS PC 3.4 | ReDMCSB | Playable, source-locked |
| **Chaos Strikes Back** | DOS PC 3.4 | ReDMCSB / CSBWin | Source-locked slices, runtime hardening |
| **Dungeon Master II: Skullkeep** | DOS | skproject | Source-locked slices, real-data hardening |
| **DM Nexus** | Sega Saturn | SH-2 disassembly | Parsing/runtime slices, real-asset handoff active |
| **Theron's Quest** | PC Engine | PC Engine disassembly and CD analysis | Parser/runtime slices; Track 02 dungeon handoff active |

## Screenshots

Only real runtime captures appear here — no mock-ups or placeholder art.

### Dungeon Master

| Original | Custom |
|----------|--------|
| ![Dungeon Master Original Hall of Champions capture at 320×200](verification-screens/pass1053-dm1-original-champion-candidate-panel/start_before_portrait_click.png) | Coming soon |

## Why Firestaff?

**Source-faithful, not guesswork.** Every gameplay-critical path is anchored to
the best available reference source code. DM1 and CSB are locked against
ReDMCSB. DM2 is locked against skproject. Nexus and Theron's Quest are verified
through Saturn SH-2 disassembly and PC Engine CD analysis respectively. The
codebase cites original source files and function names in comments so you can
trace any behavior back to its reference.

**Your data, verified.** Firestaff scans your game files by cryptographic hash,
not by filename. Put files anywhere — loose, in ZIPs, in ISO/BIN disc images —
and the scanner finds them. The checked-in hash catalog covers original assets
across all five games and their documented variants. Games with missing
required data cannot launch; there is no guessing or silent fallback.

**Two presentation paths.** Games with a verified runtime route offer
pixel-perfect **Original** rendering at the native resolution and selectable
**Custom** presentation modes with filtered, upscaled and modern targets up to
3840×2160. Custom always runs on top of the same source-locked engine — it
never bypasses collision, timing, combat or inventory logic. A game that has
not passed its launch gate is documented as bring-up work, not as a finished
playable target.

**Preservation-grade documentation.** Thousands of parity-evidence receipts,
focused tests and a [complete documentation index](docs/DOCUMENTATION_INDEX.md)
with a [comprehensive wiki](https://github.com/yeager/firestaff/wiki) covering
file formats, hardware architecture, reverse engineering details and
source-reference boundaries for all five games.

## Quick Start

1. **Download** from [GitHub Releases](https://github.com/yeager/firestaff/releases/latest).
2. **Run once** so the data directory is created.
3. **Drop your game files** anywhere under the data directory.
4. **Launch.** The scanner finds your data and shows which games are ready.

| Platform | Data directory |
|----------|---------------|
| macOS / Linux | `~/.firestaff/data/` |
| Windows | `%USERPROFILE%\.firestaff\data\` |
| iOS | Files app > On My iPhone > Firestaff > data/ |
| Android | `/sdcard/Documents/Firestaff/data/` |

Suggested layout:

```
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

## In-game runtime panel

Press **F10** in any game to open the compact runtime graphics and cheats
panel. Change presentation, filters, effects, FPS overlay, window settings and
the implemented shared cheat controls while the game is running; changes are
applied and saved immediately. Use **Up/Down** plus **Left/Right** or the
mouse, **Tab** for pages and **Esc** to close. See the [runtime panel guide](docs/runtime_graphics_and_cheats.md)
for the complete control list and source-data boundaries.

## Download

| Platform | Package |
|----------|---------|
| macOS (arm64 / x86_64) | DMG and ZIP |
| Windows (x86_64) | Installer and ZIP |
| Linux (x86_64 / arm64) | DEB and RPM |
| Steam Deck | pacman `.pkg.tar.zst` and AppImage |
| iOS (arm64) | IPA (AltStore Classic sideload) |
| Android (arm64) | APK (sideload) |

## Game Status

### Dungeon Master (PC 3.4)

Playable and source-locked against ReDMCSB. Viewport rendering, combat, spells,
movement, inventory, Hall of Champions, save/load, title sequence, entrance
animation, music, and HUD are all verified. The strongest runtime target.

### Chaos Strikes Back (PC 3.4)

Full engine coverage: DSA opcode interpreter, monster and world execution,
startup presentation, entrance and credits sequences, HUD and champion panels,
viewport geometry, thing and sensor runtime, combat and movement systems,
original save format and Utility Disk support. End-to-end playability hardening
is active.

### Dungeon Master II: Skullkeep

Source-locked against skproject across GDAT, material families, map/record
runtime, save interop, menu/title, party/inventory, spells, creature AI,
combat, world scripts and outdoor-scene slices. End-to-end V1 parity and
remaining real-data passes are still active work.

### DM Nexus (Sega Saturn)

Saturn DGN geometry, RLOWFIX.BIN resource archive, ITEM.IBS item definitions,
bounded PRS3 topology, SAL/MAP sound banks and SDDRVS.TSK sound-driver
receipts are covered. SH-2 disassembly has proven selected VDP1/VDP2 register
initialization, SCSP sound communication, FONT012 text rendering and original
IWA source-module names extracted from the binary. Positive real-asset handoff
and full visible-material playability remain active work.

### Theron's Quest (PC Engine)

JP and US Track 02 provenance is hash-verified. The CD record chain,
218-entry opaque asset manifest, startup envelope, authenticated 53-entry
descriptor receipt and SRM save boundary are covered by focused tests. A fresh authentic US Track 02 capture reaches the
System Card and BIOS CD-read path, but has not yet produced a game-owned
`$E009` data read. Object records, semantic later-level decoding,
bitmap/palette binding and the real Track 02 dungeon handoff therefore remain
open. See
[`docs/THERON_CAPTURE_READINESS.md`](docs/THERON_CAPTURE_READINESS.md).

The cross-game status and evidence boundary are kept in
[`docs/PROJECT_STATUS.md`](docs/PROJECT_STATUS.md). Presentation work follows
the same order for every game: startup, menu, HUD, then viewport.
The complete documentation map is in
[`docs/DOCUMENTATION_INDEX.md`](docs/DOCUMENTATION_INDEX.md).

## Graphics Modes

| Mode | Resolution | Description |
|------|-----------|-------------|
| **Original** | 320×200 | Pixel-faithful rendering at the original cadence and palette |
| **Custom** filtered | 640×400 – 3840×2160 | CRT-style scanlines, palette correction and sharpening |
| **Custom** upscaled | 640×400 – 3840×2160 | Clean modern output preserving the original look |
| **Custom** modern | 640×400 – 3840×2160 | New art and UI experiments |

## Command-Line Options

```
firestaff [options]
  --game <id>           Pre-select dm1, csb, dm2, nexus or theron
  --scale-mode <n>      1=Original, 2=Custom enhanced, 3=Custom modern
  --data-dir <path>     Game-data root directory
  --scan-data           Scan and report found/missing game files
  --fullscreen          Run fullscreen
  --no-vsync            Disable vertical sync
  --fps                 Show FPS counter
  --duration <ms>       Run for a fixed duration (-1 = run until exit)
  --width <px>          Window width
  --height <px>         Window height
  --script <cmds>       Comma-separated input script
  --version             Print version and exit
  --help, -h            Print help
```

Examples:

```bash
firestaff --scan-data
firestaff --game dm1 --scale-mode 1
firestaff --data-dir ~/Games/DM --fullscreen
```

## Building from Source

Requirements: CMake 3.20+, a C11 compiler, SDL3, Ninja.

```bash
brew install sdl3          # macOS
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -S . -B build -DCMAKE_C_COMPILER=cc -G Ninja
ninja -C build
./build/firestaff --help
```

Run the test suite:

```bash
ctest --test-dir build -j4 --output-on-failure
```

Some tests require original game data to pass.

### Continuous integration

Every push to `main` runs strict warnings, asset hygiene, native CMake builds
on Ubuntu, macOS and Windows, headless probes and cross-platform determinism.
The workflow cancels an older `main` run when a newer commit is pushed. A
cancelled run is not a failed test; inspect the newest run before debugging
CI. See [`docs/CI.md`](docs/CI.md) for the checks and common failure
signatures.

## Architecture

```
Launcher UI (M12)
  └─ Game engine, render loop, input and audio (M11)
       └─ Data layer, dungeon state, combat and timeline (M10)
            └─ Original game files supplied by the player
```

| Directory | Purpose |
|-----------|---------|
| `src/engine/` | SDL3 loop, game view, rendering, input, save/load, audio |
| `src/ui/` | Launcher, menu state, rendering, hit-testing |
| `src/shared/` | Asset loading, hash validation, palette, config, localization |
| `src/frontend/` | Title screens, entrance sequences, boot presentation |
| `src/memory/` | Dungeon, movement, combat, sensors, timeline, savegame model |
| `src/dm1/` | DM1 source-locked runtime |
| `src/csb/` | Chaos Strikes Back runtime |
| `src/dm2/` | Dungeon Master II runtime |
| `src/nexus/` | DM Nexus Saturn runtime |
| `src/tqr/` | Theron's Quest runtime |
| `src/dm1v2/` | DM1 Custom presentation |
| `tests/` | Thousands of integration tests and source-lock gates |
| `parity-evidence/` | Thousands of verification documents |

## The Engine in Numbers

| Metric | Count |
|--------|-------|
| Lines of C | 1,000,000+ |
| Source files | 2,600+ |
| Headers | 2,500+ |
| Test files | 3,200+ |
| Automated checks | Thousands |
| Parity-evidence documents | Thousands |
| Verified game-data hashes | Cross-game catalog |
| Localization languages | 19 |

## Source References

- [ReDMCSB](http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z) — reconstructed C source for DM1 and CSB (PC 3.4)
- [CSBWin](https://github.com/BeipDev/CSBWin) — Chaos Strikes Back reimplementation
- [skproject](https://github.com/gbsphenx/skproject) — reconstructed C source for DM2
- [Greatstone](http://greatstone.free.fr/dm/) — dungeon maps, data notes, graphics atlas
- [DMWeb](http://dmweb.free.fr/) — community documentation, file format specs, tools

## Localization

The launcher supports 19 languages via gettext PO files: English, Swedish,
German, French, Spanish, Italian, Portuguese, Dutch, Polish, Czech, Russian,
Japanese, Korean, Chinese, Danish, Norwegian, Finnish, Hungarian and Turkish.

The game-text pipeline includes 28 hand-drawn Latin Extended-A glyphs and a TTF
font cache with system-font fallback for Cyrillic, Greek, CJK and Hangul.

## Wiki

The [Firestaff Wiki](https://github.com/yeager/firestaff/wiki) has detailed
documentation for users, developers and preservationists:

The repository's [project status](docs/PROJECT_STATUS.md) is the canonical
short summary; the wiki expands each game's formats, hardware and reverse-
engineering evidence without changing the playability claims.

- Per-platform install guides (macOS, Windows, Linux, iOS AltStore, Android sideload)
- Reverse engineering documentation for all five games (function registries, file formats, data structures)
- File format specifications: DUNGEON.DAT, GRAPHICS.DAT (IMG3/IMG1/GDAT), save formats
- Technical references and source-lock architecture
- Deep internals: DM1 PC34, CSB DSA, DM2 GDAT, Nexus DGN/PRS3/Saturn hardware, Theron's Quest Track 02
- Preservation worklists for ReDMCSB, CSBWin and skproject

## Legal

Firestaff is a clean-room engine reimplementation. You need original game files
that you legally own. No copyrighted game data is included.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of FTL
Games. DM Nexus is a trademark of Victor Interactive Software. Theron's Quest is
a trademark of Working Designs / Victor Interactive Software.

## License

MIT. See [LICENSE](LICENSE).
