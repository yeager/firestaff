# Firestaff

Firestaff is a clean-room engine for the Dungeon Master games. It uses the
original files supplied by the player and never bundles game data.

[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff" width="360">
</p>

## What works today

Dungeon Master for PC DOS 3.4 is the complete, playable route: startup,
dungeon, HUD, input and saves all run from original data.

The other games are being brought up against their original media. A recognised
edition in the launcher means that Firestaff found and verified its files. It
does not mean that its whole campaign is finished.

| Game | Current scope |
|---|---|
| Dungeon Master | Playable PC DOS 3.4 route. |
| Chaos Strikes Back | Real-data startup, dungeon, utility, HUD, input and save work is under active integration. |
| Dungeon Master II: Skullkeep | Boot, utility, presentation and runtime systems are under active development. |
| DM Nexus | Sega Saturn data, world and rendering work is in progress. |
| Theron's Quest | PC Engine CD data and runtime work is in progress. |

For the detailed, current boundary of each game, see
[Project status](docs/PROJECT_STATUS.md).

## Original editions

Firestaff recognises original editions by the contents of their files, not by
their filenames. Keep a game in the folder layout you already use; the scanner
searches below the selected data directory and checks the files it finds.

| Game | Editions recognised |
|---|---|
| Dungeon Master | PC DOS 3.4, Atari ST, Amiga and FM Towns |
| Chaos Strikes Back | PC DOS 3.4, Atari ST 2.0/2.1, Amiga A31/A35 and FM Towns |
| Dungeon Master II: Skullkeep | DOS, PC-9801/9821, FM Towns and Amiga catalogued editions |
| DM Nexus | Sega Saturn |
| Theron's Quest | PC Engine CD, Japanese and US Track 02 media |

Each edition keeps its own data, startup sequence and save format. Firestaff
does not fill a missing Atari, Amiga or FM Towns asset with a PC substitute.
That distinction matters especially for Chaos Strikes Back:

- Atari ST has its own animation and game-data route.
- Amiga A31 and A35 have different program and title hand-offs.
- FM Towns has separate English and Japanese Game and Utility programs, plus
  its own F31 save container.

Availability depends on the exact files supplied. The launcher explains when a
recognised edition still lacks the files required for its native route.

## Getting started

1. Put legally owned original game files under Firestaff's data directory, or
   choose a different directory in the launcher.
2. Scan the directory.
3. Launch an available game.

The default data directory is:

```text
~/.firestaff/data/
```

For example:

```text
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

Those names are only a convenience. Firestaff searches recursively, and never
requires you to rename your original files.

```bash
firestaff --scan-data
firestaff --data-dir /path/to/games --scan-data
firestaff --game dm1
```

Incomplete or mismatched data is blocked before launch. Optional title,
animation and utility files can enable more of a route, but cannot replace the
required game files.

## Platforms and presentation

Firestaff builds for macOS, Windows and Linux. The original editions listed
above are game-data targets, not separate host-platform builds.

Where a route is ready, Firestaff offers the original 320×200 presentation and
optional filtered, upscaled and modern display modes. These modes use the same
original-data gate and game state.

## Build from source

Firestaff is written in C11 and uses CMake and SDL3.

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/firestaff --scan-data
```

On macOS, install SDL3 with Homebrew:

```bash
brew install sdl3
```

Run the test suite with:

```bash
ctest --test-dir build --output-on-failure
```

Some integration tests need a local corpus of original game files. Continuous
integration does not use or distribute game data.

## Documentation

- [Project status](docs/PROJECT_STATUS.md)
- [Game-data setup](docs/DATA_SETUP.md)
- [Documentation index](docs/DOCUMENTATION_INDEX.md)
- [Firestaff wiki](https://github.com/yeager/firestaff/wiki)

## Legal

Firestaff contains no copyrighted game data. You need original files that you
own legally.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of FTL
Games. DM Nexus is a trademark of Victor Interactive Software. Theron's Quest
is associated with Working Designs and Victor Interactive Software.

## License

[MIT](LICENSE).
