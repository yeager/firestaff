# Firestaff

Firestaff is a clean-room engine project for the Dungeon Master family of
games. It reads original game media supplied by the player, identifies it by
content hash and keeps platform-specific behaviour tied to the best available
source and format references.

Dungeon Master for PC DOS 3.4 is the current playable route. Chaos Strikes
Back, Dungeon Master II, DM Nexus and Theron's Quest are under active
development. Their presence in the launcher means that the supplied media was
recognised, not that every campaign path is complete.

[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff" width="360">
</p>

## What you can use today

| Game | Current state |
|---|---|
| Dungeon Master | Playable, source-locked PC DOS 3.4 route with original-data startup, dungeon, HUD, input and saves. |
| Chaos Strikes Back | Active real-data work. The launcher, data scanner, startup, utility, dungeon, HUD and save paths are being brought together across the original editions. Do not treat it as a finished campaign yet. |
| Dungeon Master II: Skullkeep | Active real-data work on boot, utility, presentation and runtime systems. |
| DM Nexus | Active Saturn-data bring-up. |
| Theron's Quest | Active PC Engine CD-data bring-up. |

The concise, current boundary for every game is in
[Project status](docs/PROJECT_STATUS.md). The data required for each route is
listed in [Game-data setup](docs/DATA_SETUP.md).

## Original editions

Firestaff scans recursively. Keep your files in the layout that suits your
collection: loose files, supported archives and supported disc images can all
be discovered without renaming them. Required files are matched by hash, so a
name alone never enables a launch.

| Game | Original editions recognised by the data scanner | Notes |
|---|---|---|
| Dungeon Master | PC DOS 3.4, Atari ST, Amiga and FM Towns | PC DOS 3.4 is the playable route. The other editions are retained as real media sources for their own startup, graphics and format work. |
| Chaos Strikes Back | PC DOS 3.4, Atari ST 2.0/2.1, Amiga A31/A35 and FM Towns | PC DOS 3.4 is the main runtime route. Atari, Amiga and FM Towns each have distinct files, startup programs and save formats; Firestaff does not substitute one platform's assets for another's. |
| Dungeon Master II: Skullkeep | DOS, PC-9801/9821, FM Towns and Amiga editions covered by the catalog | Recognition and runtime coverage vary by edition. |
| DM Nexus | Sega Saturn media | Keep the complete disc image and associated data together. |
| Theron's Quest | PC Engine CD Japanese and US Track 02 media | Keep the original CUE/BIN set together when available. |

For Chaos Strikes Back, FM Towns has separate English and Japanese Game and
Utility programs. Its native F31 save reader accepts only a verified original
save container. Atari and Amiga saves remain their own native formats. This
is deliberate: copying a PC save or graphic into another edition would hide a
compatibility error rather than preserve the game.

## Getting started

1. Obtain original media that you own legally.
2. Start Firestaff once, then place the media under its data directory or
   choose another directory in the launcher.
3. Run a scan. A game is available only when the required hashes form a valid
   matching set.

The default data directory is:

```text
~/.firestaff/data/
```

A convenient layout is:

```text
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

The subdirectories are optional. Firestaff searches below the selected root.
Never add game files, save files or disc images to this repository.

```bash
firestaff --scan-data
firestaff --data-dir /path/to/games --scan-data
firestaff --game dm1
```

The scanner reports missing requirements clearly and blocks an incomplete or
mismatched launch. Optional title, animation, utility and save files can add
capabilities to an edition, but they do not replace its required graphics and
dungeon data.

## Platforms and presentation

Firestaff builds for macOS, Windows and Linux. The packaged and runtime-tested
scope differs by game, so use the game status above rather than assuming that
a package makes every original edition playable.

The engine offers the original 320×200 presentation where the source route is
ready, plus optional filtered, upscaled and modern presentation modes. Those
modes sit on top of the same game state and data gates; they are not a
replacement for the original media.

## Controls and launcher

The launcher lets you select a game, graphics mode and data directory. It
also reports which editions it found before a launch is attempted. In a live
game, F10 opens the runtime panel for presentation options and the shared
controls that are implemented for that route.

Theron's Quest controls are documented in
[Theron input](docs/theron_input.md): W/S move forward/back, A/D turn, mouse
buttons 1/2 are Button I/II, and short/long touch uses the same pair.

Command-line options:

```text
firestaff [options]
  --game <id>           Select dm1, csb, dm2, nexus or theron
  --data-dir <path>     Game-data root
  --scan-data           Scan recognised original media
  --scale-mode <n>      Select presentation mode
  --fullscreen          Run fullscreen
  --no-vsync            Disable vertical sync
  --fps                 Show the frame-rate counter
  --duration <ms>       Run for a fixed duration
  --width <px>          Window width
  --height <px>         Window height
  --script <cmds>       Comma-separated input script
  --version             Print the version
  --help, -h            Show help
```

## Build from source

Firestaff is C11 with CMake and SDL3.

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/firestaff --scan-data
```

On macOS, SDL3 is available through Homebrew:

```bash
brew install sdl3
```

Run tests with:

```bash
ctest --test-dir build --output-on-failure
```

Some integration tests require a local, legally owned original-data corpus.
Continuous integration checks warnings, build configurations, headless probes
and determinism without committing game data to the repository.

## How the project is built

```text
M12  launcher and game-data selection
M11  game view, rendering, input and audio
M10  dungeon data, movement, combat, sensors and timeline
      original files supplied by the player
```

DM1 and CSB work is cross-referenced against ReDMCSB. CSBWin, DMWeb and
Greatstone are used for additional CSB and file-format evidence. DM2 work
uses skproject, while Nexus and Theron's Quest are tied to their respective
original-media and hardware references. Source comments identify the relevant
function or format boundary where that helps future maintenance.

Useful documentation:

- [Project status](docs/PROJECT_STATUS.md)
- [Game-data setup](docs/DATA_SETUP.md)
- [CI notes](docs/CI.md)
- [Documentation index](docs/DOCUMENTATION_INDEX.md)
- [Firestaff wiki](https://github.com/yeager/firestaff/wiki)

## Legal

Firestaff contains no copyrighted game data. You need original files that you
own legally.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of
FTL Games. DM Nexus is a trademark of Victor Interactive Software. Theron's
Quest is associated with Working Designs and Victor Interactive Software.

## License

[MIT](LICENSE).
