# Firestaff

**A source-faithful Dungeon Master engine for modern hardware.**

Firestaff brings the classic FTL dungeon crawlers to macOS, Windows,
Linux and Steam Deck era machines while keeping the original game logic
anchored to the best available source references. It runs original game
data you already own, validates it by hash, and lets you choose between
pixel-perfect presentation, 640x400 filtered V2.0 output, and selectable
V2.1/V2.2 presentation resolutions from 640x400 up to 3840x2160.

[![Release](https://img.shields.io/github/v/release/yeager/firestaff)](https://github.com/yeager/firestaff/releases/latest)
[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Steam%20Deck-orange)]()

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## Screenshots

| Original-style DM1 | Enhanced title rendering |
|---|---|
| ![DM1 original-style dungeon view](verification-screens/01_ingame_start_latest.png) | ![Enhanced title rendering](docs/compare/v21/title.png) |

| V1 title source path | V2 4K presentation work |
|---|---|
| ![V1 title rendering](docs/compare/v1/title.png) | ![V2 4K presentation capture](verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png) |

## Current Status

Firestaff is in active development. The launcher, data scanner, build
system, packaging scripts and source-lock verification framework are in
place. DM1 is the strongest runtime target today. The other games have
hash-verified launch profiles and substantial engine slices, with end-to-end
playability still being hardened game by game.

The table below separates runtime fidelity from presentation work. A
"verified slice" means the named behavior is covered by focused tests or
probes; it is not a claim that the whole game is finished.

Across the game profiles that expose V2.0, filtered mode targets a fixed
640x400 runtime surface. It presents the original 320x200 framebuffer at 2x
scale and maps input back to the original game coordinate space.

V2.1 and V2.2 profiles expose a per-game resolution setting. The launcher
offers targets from 640x400 through 4K/3840x2160; M11 renders the selected
presentation surface while preserving the original 320x200 input/game
coordinate space.

| Game / version | Current status |
|---|---|
| DM1 V1 Original | Playable and source-locked against the PC 3.4 lineage. Recent work hardened viewport evidence drift, wall/door/floor source-lock gates, chest and inventory runtime behavior, champion-panel pixels, mirror/candidate routes, capture tooling, and verifier stability. Remaining work is mainly paired original-DOS capture proof and visual polish. |
| DM1 V2.0 / V2.1 / V2.2 | V2.0 filtered presentation is fixed at 640x400. V2.1/V2.2 expose per-game selectable presentation resolution from 640x400 through 3840x2160, with V1 command ownership preserved. Side-by-side V1/V2 seed and region manifests are in place; full enhanced screenshot/pixel gates and broader presentation polish remain active. |
| CSB V1 Original | Hash-verified launch/profile boundary, real DUNGEON.DAT load, dungeon handle handoff, object-chain access, imported champion stats/load behavior, party rotation, tick accumulation, timeline dispatch, wall text, and deterministic boot-to-viewport render slices are verified. End-to-end CSB playability, title/import UI composition, broader command queue binding, and full viewport integration are still being hardened. |
| CSB V2.0 / V2.1 / V2.2 | V2.0 filtered presentation uses the shared 640x400 target, and V2.1/V2.2 share the selectable 640x400-to-4K presentation-resolution setting where exposed. V1 compatibility and launch/profile separation exist. HUD overlay and smooth-movement scaffolds are covered by probes, but enhanced assets, lighting, controller ergonomics, and full side-by-side screenshot verification remain open. |
| DM2 V1 Original | Boot/profile, utility/import, world-state, save/load, weather, projectile-door, asset-loader, dungeon-loader, object-model, and map-state probes exist. Broader dungeon, rendering, mechanics, creature/combat, shops/NPCs, and real-runtime compatibility remain active work. |
| DM2 V2.0 / V2.1 / V2.2 | V2.0 filtered presentation uses the shared 640x400 target, and V2.1/V2.2 share the selectable 640x400-to-4K presentation-resolution setting where exposed. Enhanced asset, HUD, lighting/outdoor effects, smooth movement, touch/controller, and verification scaffolds are implemented. V2 remains presentation work on top of the still-active V1 parity effort. |
| Nexus V1 Original | Saturn DMDF/DGN parsing, world/runtime state, rendering slices, save/load, actor bounds, mechanics scaffolding, and verification paths exist. Launcher/game-loop handoff with real Saturn asset-path proof and broader runtime coverage remain active. |
| Nexus V2.0 / V2.1 / V2.2 | V2.0 filtered presentation uses the shared 640x400 target where the enhanced path is exposed, and V2.1/V2.2 share the selectable 640x400-to-4K presentation-resolution setting once launchable. Asset, UI, lighting, and touch/controller slices exist, but V2 compatibility lock, launch/profile separation, smooth movement, and full verification remain behind the V1 handoff proof. |
| Theron's Quest V1 Original | JP/US Track 02 provenance is hash-verified. Parser, world/progression state, viewport/UI, initial mechanics, save/load, shop-table guards, direct hash-verified boot-profile loading, and a narrow US bank-boundary signal are verified. Exact Track 02 dungeon-bank offsets, full dungeon loader parity, runtime playability path, and README-eligible real screenshots remain active work. |
| Theron's Quest V2.0 / V2.1 / V2.2 | Not started beyond keeping the V1 compatibility boundary honest. When a V2.0 path is exposed, it should use the same 640x400 filtered target as the other games. V2 work waits on stronger V1 Track 02 parity and runtime proof. |

## What Firestaff Gives You

- **Source-faithful gameplay work**: DM1 and related systems are checked
  against ReDMCSB, CSBWin, skproject, CSB lineage sources and Greatstone data
  references.
- **Modern launcher**: a 1920x1080 start menu with per-game status, settings,
  language support, availability checks and hash-verified data discovery.
- **Game-data scanning**: Firestaff searches recursively by file hash, not by
  filename or folder layout. It lists required files that are found or missing.
- **Launch safety**: games with missing required data are shown as unavailable
  and cannot be started until the required hashes are present. Optional title,
  intro and other non-essential extras can be absent.
- **Multiple presentation modes**: original V1 rendering, filtered V2.0 at
  640x400, and selectable V2.1/V2.2 presentation targets up to 3840x2160.
- **Cross-platform C11 engine**: pure C, CMake, SDL3, no C++ dependency.
- **Packaging path**: preview packaging scripts exist for macOS DMG/ZIP,
  Windows ZIP/installer and Linux DEB/RPM.

## Latest Release

**Current version:** `2.7.13`

The latest release is a DM1 V1 combat fidelity and bug audit release,
documented in `docs/DM1_V1_BUG_AUDIT.md`:

- Armor defense now uses the F0321 wound defense calculation with per-slot
  iteration and (130 − avgDefense) / 64 attack scaling.
- Fire Shield, Spell Shield, and Psychic damage types now apply per
  CHAMPION.C F0321. Creature melee poison uses F0307 vitality adjustment.
- 7 creature types promoted from STUB to FULL: Giant Scorpion, Giggler,
  Screamer, Vexirk, Magenta Worm, Animated Armour, Red Dragon. 10 of 27
  creature types now FULL.
- Luck and stamina adjustments, creature attack ordering, flee behavior,
  projectile sub-cell hit mask, Thieves Eye, light table, stat gain cycle,
  magic map per-champion state, runtime dynamics table, and savegame
  field mask semantics all source-locked to ReDMCSB.
- Test infrastructure: FIRESTAFF_BUILD_DIR env var for out-of-tree builds.

See [RELEASE_NOTES.md](./RELEASE_NOTES.md) for the release history.

## Download

Get the latest build from [GitHub Releases](https://github.com/yeager/firestaff/releases/latest).

| Platform | Package |
|---|---|
| macOS | DMG and ZIP |
| Windows | Installer and ZIP |
| Linux x86_64 | DEB and RPM |
| Linux ARM64 / Steam Deck | DEB and RPM |

All game data is user-supplied. Firestaff does not include copyrighted game
assets.

## Quick Start

1. Download Firestaff from [Releases](https://github.com/yeager/firestaff/releases/latest).
2. Run it once so the default data directory is created.
3. Put your original game files anywhere under the configured data directory.
4. Start Firestaff. The launcher scans the directory automatically and shows
   which games are ready.

Default data directory:

```text
~/.firestaff/data/
```

Suggested subdirectories:

```text
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

Filenames are less important than file hashes. Firestaff searches
recursively, so a custom folder layout works as long as the original files are
present. Game data may also live inside ZIP archives or ISO/BIN disc images;
the scanner hashes archive contents and reports matches as virtual paths.

## Game Data Scanner

Use the CLI scanner to see what Firestaff can find:

```bash
firestaff --scan-data
```

or:

```bash
firestaff --scan-game-data
```

The scanner reports required data per game. Required files block launch when
missing. Non-essential extras such as title or intro animation files are
reported as optional and can be skipped.

ZIP files are supported for hash discovery across the game-data root. Stored
entries are supported everywhere; deflated entries are supported when Firestaff
is built with zlib, which is enabled automatically when CMake finds it. ISO/BIN
disc images are scanned as ISO 9660 containers, covering DM2 disc images and
the existing Saturn/Nexus data-image path. For DM1, CSB, and DM2, required
files found inside archives are materialized into Firestaff's local asset cache
before launch so the runtime still receives ordinary game-data paths.

You can point Firestaff at a custom root:

```bash
firestaff --data-dir ~/Games/FirestaffData --scan-data
```

The launcher also exposes the configured data directory and game availability
in the start menu.

## Command-Line Options

```text
firestaff [options]
  --duration <ms>       Run for a fixed duration (-1 = run until exit)
  --width <px>          Window width
  --height <px>         Window height
  --scale-mode <n>      1=V1 original, 2=V2.1 enhanced, 3=V2.2 modern
  --script <cmds>       Comma-separated input script
  --data-dir <path>     Game-data root
  --scan-data           Scan game data and print found/missing files
  --scan-game-data      Alias for --scan-data
  --game <id>           Pre-select dm1, csb, dm2, nexus or theron
  --fullscreen          Run fullscreen
  --no-vsync            Disable vertical sync
  --fps                 Show FPS counter
  --version             Show version and exit
  --help, -h            Show help
```

Examples:

```bash
firestaff --scan-data
firestaff --game dm1 --scale-mode 1
firestaff --data-dir ~/Games/FirestaffData --fullscreen
firestaff --duration 5000 --fps
```

## Graphics Modes

| Mode | Resolution target | Purpose |
|---|---|---|
| V1 Original | 320x200 | Pixel-faithful original rendering |
| V2.0 Filtered | 640x400 | 2x presentation of the original framebuffer with CRT scanlines, palette correction and sharpening |
| V2.1 Upscaled | Selectable 640x400 to 3840x2160 | Cleaner modern output while preserving the DM look |
| V2.2 Modern | Selectable 640x400 to 3840x2160 | New modern art and UI experiments |

V1 owns gameplay-critical behavior. V2 modes are presentation layers and must
not bypass source-locked command, collision, timing or inventory routes.

## Architecture

```text
M12  Modern launcher UI
  -> M11 game engine, render loop, input and audio
      -> M10 data layer, dungeon state, graphics, combat and timeline
          -> Original game files supplied by the player
```

Main source areas:

| Directory | Purpose |
|---|---|
| `src/engine/` | SDL loop, game view, rendering, input, save/load and audio |
| `src/ui/` | Modern launcher, menu state, rendering and hit-testing |
| `src/shared/` | Asset loading, hash validation, palette, config and localization |
| `src/frontend/` | Title screens, entrance sequences and boot presentation |
| `src/memory/` | Dungeon, movement, combat, sensors, timeline and savegame model |
| `src/dm1/` | DM1 source-locked runtime systems |
| `src/dm1v2/` | DM1 enhanced presentation systems |
| `src/csb/` | Chaos Strikes Back runtime and presentation work |
| `src/dm2/` | Dungeon Master II runtime and presentation work |
| `src/nexus/` | Saturn DM Nexus DMDF/DGN support |
| `src/theron/` | Theron's Quest support |
| `tests/` and `probes/` | Integration tests, source-lock gates and headless probes |

## Source Fidelity

Firestaff is built around source references, not guesswork.

Primary references:

- [ReDMCSB](http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z) for DM1 and
  large parts of the PC-34 lineage.
- [CSBWin](https://github.com/BeipDev/CSBWin) and
  [CSB](https://github.com/zelurker/CSB) for Chaos Strikes Back lineage.
- [skproject](https://github.com/gbsphenx/skproject) for Dungeon Master II.
- [Greatstone](http://greatstone.free.fr/dm/) for dungeon maps, data notes and
  graphics atlas material.

Source-lock comments in the code cite the relevant original source files and
functions for gameplay-critical behavior.

## Building from Source

Requirements:

- CMake 3.20 or newer
- A C11 compiler
- SDL3

On macOS:

```bash
brew install sdl3
```

Build:

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/firestaff --help
```

Run the headless Phase A probe:

```bash
SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe
```

Run the full local test set:

```bash
ctest --test-dir build --output-on-failure
```

Some integration tests need original game data.

## Localization

The launcher uses gettext PO files and supports a broad language set, including
English, Swedish, German, French, Spanish, Italian, Portuguese, Dutch, Polish,
Czech, Russian, Japanese, Korean, Chinese, Danish, Norwegian, Finnish,
Hungarian and Turkish.

## Legal

Firestaff is a clean-room engine reimplementation based on public source and
format references. You need original game data files that you legally own.

No copyrighted game data is included.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of FTL
Games. DM Nexus is a trademark of Victor Interactive Software. Theron's Quest
is a trademark of Working Designs / Victor Interactive Software.

## License

MIT. See [LICENSE](LICENSE).
