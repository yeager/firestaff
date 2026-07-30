# Firestaff

**A source-faithful Dungeon Master engine for modern hardware.**

Firestaff brings the classic FTL dungeon crawlers to macOS, Windows,
Linux and Steam Deck era machines while keeping the original game logic
anchored to the best available source references. It runs original game
data you already own, validates it by hash, and lets you choose between
pixel-perfect **Original** presentation and the selectable **Custom**
presentation targets from 640x400 up to 3840x2160.

> ℹ️ The "Original" and "Custom" labels in this README correspond to the
> internal `V1` and `V2.0`/`V2.1`/`V2.2` code paths. Outside this file the
> internal `v1`/`v2` naming is still used in the codebase, AGENTS.md,
> CMake flags and the CLI `--scale-mode` argument.

[![Release](https://img.shields.io/github/v/release/yeager/firestaff)](https://github.com/yeager/firestaff/releases/latest)
[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platforms](https://img.shields.io/badge/platforms-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Steam%20Deck-orange)]()

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## Screenshots

These are captured from Firestaff's runtime, not generated mock-ups. **Original
(V1)** uses the native 320x200 coordinate space and source palette. **Custom
(V2.x)** uses the same gameplay state with the selected presentation pipeline.

### Dungeon Master

| Original (V1), runtime dungeon and HUD | Custom (V2.x), runtime dungeon presentation |
|---|---|
| ![Dungeon Master Original V1 runtime capture at 320x200](verification-screens/07_party_hud_with_champions.png) | ![Dungeon Master Custom V2.x runtime capture at 3840x2160](verification-screens/v2-initial-4k/firestaff-v2-initial-ingame-4k.png) |

| Original (V1), title palette | Custom (V2.1), filtered title |
|---|---|
| ![Dungeon Master Original V1 title capture](docs/compare/v1/title.png) | ![Dungeon Master Custom V2.1 title capture](docs/compare/v21/title.png) |

### Capture Coverage

| Game | Original (V1) | Custom (V2.x) |
|---|---|---|
| Dungeon Master | Published runtime captures | Published runtime captures |
| Chaos Strikes Back | Capture pending verified package-data session | Capture pending verified package-data session |
| Dungeon Master II: Skullkeep | Capture pending verified GDAT session | Capture pending verified GDAT session |
| Theron's Quest | Capture pending verified Track 02 session | Capture pending verified Track 02 session |
| DM Nexus | Capture pending verified Saturn package session | Capture pending verified Saturn package session |

The remaining games are intentionally not illustrated with placeholder art.
Their screenshots will be added only after each title has a repeatable,
data-backed runtime capture.

## Current Status

Firestaff is in active development. The codebase contains ~905k lines of C
across 2,406 source files, 2,372 headers, 2,892 test files and 765 probe
files. The test suite has 3,624 tests. 165 verified game-data hashes cover
original assets across all five games. The launcher, data scanner, build
system, packaging scripts and source-lock verification framework are in
place. **DM1 Original** is the strongest runtime target today. The
other games have hash-verified launch profiles and substantial engine
slices, with end-to-end playability still being hardened game by game.

The table below separates runtime fidelity from presentation work. A
"verified slice" means the named behavior is covered by focused tests or
probes; it is not a claim that the whole game is finished.

For each game, **Original (V1)** is the pixel-faithful, source-locked
runtime. **Custom (V2.0..V2.2)** is the presentation family that runs
on top of the same engine: filtered 2x mode, AI-upscaled 10x mode, and a
modern art experiment. The three Custom modes share one selectable
resolution that the launcher offers from 640x400 through 4K
(3840x2160). Custom always keeps the Original command, collision,
timing and inventory routes.

| Game | Original (V1) | Custom (V2.0..V2.2) |
|---|---|---|
| **Dungeon Master (DM1)** | Playable and source-locked against the PC 3.4 lineage (ReDMCSB). Current work focuses on visual parity, combat/spell edge cases, Hall of Champions reliability, save compatibility and packaged-release smoke testing. Q-DM1-01 through Q-DM1-10 complete. | Selectable Custom modes exist from filtered 2x through 4K-oriented presentation. They run on top of the Original runtime and remain presentation work, with screenshot/pixel gates and art polish still active. |
| **Chaos Strikes Back (CSB)** | Hash-verified launch/profile boundary, real dungeon load, object-chain access, imported champion behavior, party rotation, timeline dispatch, wall text and deterministic boot-to-viewport slices are verified. DSA opcode core, monster/world execution, startup presentation, entrance/credits, HUD/champion panels, viewport geometry, thing/sensor runtime, combat/movement, original saves/Utility Disk, and media/input/expansion all complete (Q-CSB-01 through Q-CSB-10). End-to-end playability is still being hardened. | Custom mode shares the same selectable resolution path as DM1. HUD overlay, smooth movement scaffolds and runtime handoff slices exist; enhanced assets and full viewport verification remain open. |
| **Dungeon Master II: Skullkeep (DM2)** | Source-locked against skproject. GDAT core renderer (76 tests), material families, creature renderer (22 tests), G1 map/c_record runtime (19 tests), SKSAVE interop, menu/title/audio, party/inventory/spells, creature AI/combat, CCM/world scripts, and outdoor scenes all complete (Q-DM2-01 through Q-DM2-10). Real-data gate repair (v3.0.181) reduced DM2 failures from 13 to 2. Active work: SkWinCore symbol audit, real-data creature/combat passes, and audible playback backend. | Enhanced asset, HUD, lighting/outdoor effects, smooth movement, touch/controller and verification scaffolds are implemented. Custom remains presentation work on top of the still-active Original parity effort. |
| **DM Nexus (Saturn)** | Saturn DMDF/DGN parsing, world/runtime state, rendering slices, save/load, actor bounds, mechanics scaffolding and verification paths exist. Structure1B/1F/2/3 geometry, PRS3 archive topology, SLEV task receipts, and SAL/MAP container routes are proven. Launcher/game-loop handoff with real Saturn asset-path proof remains active work. | Custom selection and asset/UI/lighting scaffolds exist, but full Custom compatibility depends on the Original handoff proof. |
| **Theron's Quest** | JP/US Track 02 provenance is hash-verified. Strict CUE/BIN handoff, IPL/Stage Two record chain, 218-entry manifest, startup level envelope, and SRM gzip boundary are proven. Parser, world/progression state, viewport/UI, mechanics, save/load, shop-table guards and direct boot-profile loading have focused coverage. Full dungeon-loader parity and broader playability proof remain active work. | Custom selection, settings, filter/upscale/modern scaffolding and smooth-movement gates are verified. Finished real art, enhanced UI overlays and screenshot/material pixel gates remain active work. |

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
- **Hash-based direct launch**: `--data-dir <path>` boots the supplied
  variant via MD5-hash discovery in `m11_resolve_builtin_dungeon_path`
  + first-matched-version fallback in `M11_GameView_OpenSelectedMenuEntry`,
  so any non-default variant the scanner recognises (e.g. Theron US
  Track 02, CSB Amiga 3.3 Meynaf FR, DM1 legacy-dos PC34) launches
  without manual versionIndex tweaking. Tier 1 #5 strict boot-probe
  (`firestaff_tier1_strict_boot_probe` ctest entry) currently verifies
  all present in-scope paths reach their boot milestone.
- **Two presentation families per game**: pixel-faithful **Original** at
  320x200, and **Custom** with selectable filtered, upscaled, and modern
  targets up to 3840x2160.
- **Cross-platform C11 engine**: pure C, CMake, SDL3, no C++ dependency.
  ~905k lines of C, 3,624 tests, 935 parity-evidence documents.
- **Packaging path**: preview packaging scripts exist for macOS DMG/ZIP,
  Windows ZIP/installer and Linux DEB/RPM.

## Latest Release

**Current version:** `3.0.181`.

Release-specific details live in
[GitHub Releases](https://github.com/yeager/firestaff/releases/latest). The
README keeps the stable project status, build instructions, data-scanner
behavior and platform notes in one place instead of duplicating release notes.

## Download

Get the latest build from [GitHub Releases](https://github.com/yeager/firestaff/releases/latest).

| Platform | Package |
|---|---|
| macOS | DMG and ZIP |
| Windows | Installer and ZIP |
| Linux x86_64 | DEB and RPM |
| Steam Deck (x86_64 / SteamOS) | pacman `.pkg.tar.zst` |
| Linux ARM64 | DEB and RPM |

All game data is user-supplied. Firestaff does not include copyrighted game
assets.

## Quick Start

1. Download Firestaff from [Releases](https://github.com/yeager/firestaff/releases/latest).
2. Run it once so the default data directory is created.
3. Put your original game files anywhere under the configured data directory.
4. Start Firestaff. The launcher scans the directory automatically and shows
   which games are ready.

Default data directory on macOS and Linux:

```text
~/.firestaff/data/
```

Default data directory on Windows:

```text
<Firestaff installation folder>\data\
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

Linux users can also read the full manual page source at
[`docs/linux/firestaff.1`](docs/linux/firestaff.1).

```text
firestaff [options]
  --duration <ms>       Run for a fixed duration (-1 = run until exit)
  --width <px>          Window width
  --height <px>         Window height
  --scale-mode <n>      1=Original, 2=Custom enhanced, 3=Custom modern
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

> ℹ️ The `--scale-mode` numbers `1`/`2`/`3` are stable across releases and
> match the internal `V1`/`V2.1`/`V2.2` code paths. `1` is the pixel-faithful
> **Original** mode; `2` and `3` are the two **Custom** presentation modes.

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
| **Original** (V1) | 320x200 | Pixel-faithful original rendering at the DM PC 3.4 cadence |
| **Custom** filtered (V2.0) | 640x400..3840x2160 (user-selectable) | 2x presentation of the original framebuffer with CRT scanlines, palette correction and sharpening |
| **Custom** upscaled (V2.1) | 640x400..3840x2160 (user-selectable) | Cleaner modern output while preserving the DM look |
| **Custom** modern (V2.2) | 640x400..3840x2160 (user-selectable) | New modern art and UI experiments |

**Original** owns gameplay-critical behavior. **Custom** modes are
presentation layers and must not bypass source-locked command, collision,
timing or inventory routes.

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

The M11 game-text pipeline covers non-ASCII characters that the original
engine did not have glyphs for:

- **28 hand-drawn Latin Extended-A glyphs** (Ä Ö Å Ü ß é è ê ç à â î ï ô û
  ñ ã õ ü ï ø) plus a UTF-8 decoder. Restores 244 of 548 (44%) of
  `sv.po` msgstrs that previously rendered as SPACE.
- **TTF font cache** (`firestaff_font_cache_pc34_compat.c`): per-language
  TTF lookup chain covering all 19 l10n languages with
  `<asset>/fonts/NotoSans-<lang>.ttf`, system fallback (`Arial
  Unicode.ttf` on macOS, DejaVu on Linux, Arial on Windows), and CJK
  fallback (`NotoSansCJK` / `Hiragino Sans GB`). Used by the SDL3_ttf
  renderer to cover Cyrillic, Greek, Kanji, Hangul, and CJK beyond what
  the bitmap-glyph table supports.

## Legal

Firestaff is a clean-room engine reimplementation based on public source and
format references. You need original game data files that you legally own.

No copyrighted game data is included.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of FTL
Games. DM Nexus is a trademark of Victor Interactive Software. Theron's Quest
is a trademark of Working Designs / Victor Interactive Software.

## License

MIT. See [LICENSE](LICENSE).
