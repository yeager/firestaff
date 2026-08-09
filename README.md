# Firestaff

Firestaff is a clean-room engine project for the Dungeon Master games. It
uses game files you already own, identifies them by content hash and keeps the
original files separate from the engine.

Dungeon Master for PC DOS 3.4 is the current playable route. Work on Chaos
Strikes Back, Dungeon Master II, DM Nexus and Theron's Quest is active, but
those games are not described here as finished releases.

[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## What works today

The PC DOS 3.4 version of Dungeon Master has the most complete route: startup,
menus, dungeon view, HUD, controls, combat and save handling run from original
data. It is still being checked against original captures and save material.

The other games have real-data scanners, source references and individual
runtime paths, but their remaining boundaries matter. The concise, current
description is kept in [project status](docs/PROJECT_STATUS.md).

| Game | Current scope |
|---|---|
| Dungeon Master | Playable PC DOS 3.4 route; further parity work continues. |
| Chaos Strikes Back | Real-data startup, dungeon, utility, input and rendering work is under active hardening. |
| Dungeon Master II: Skullkeep | Engine and data work in progress. |
| DM Nexus | Saturn real-data bring-up in progress. |
| Theron's Quest | PC Engine real-media bring-up in progress. |

## Chaos Strikes Back editions

Firestaff recognises original CSB editions by hash rather than by their folder
names. The scanner currently covers the following families when the required
matching data is present:

| Original family | What Firestaff does with it today |
|---|---|
| PC DOS 3.4 | Main CSB source-reference path and the basis for the ongoing campaign runtime work. |
| Atari ST 2.0 and 2.1 | Scanned as native Atari media; startup, animation and original-save handoff work are covered by dedicated runtime paths. |
| Amiga 3.1 and 3.5 | Scanned as native Amiga media; platform-specific title and presentation routes are being hardened. An edition without a verified native program handoff is blocked instead of borrowing assets from another release. |
| FM Towns English and Japanese | Scanned as native CD installations; title, game and Utility routes use the original Towns data where the required package is available. |

Recognition is deliberately separate from a playability claim. A recognised
edition has passed the data gate; it does not imply that every screen, save
format or gameplay path has reached parity. Firestaff does not substitute
files from a different edition to make a route appear to work.

## Your game data

No game data is included. Keep your legally owned files in any directory and
tell Firestaff where to look. It searches recursively and can inspect supported
loose files, ZIP archives and disc-image containers without relying on
filenames.

For the DAT-based games, `GRAPHICS.DAT` and `DUNGEON.DAT` must come from the
same original edition. The launcher rejects incomplete or mismatched pairs.
Optional title, animation, music and save files stay useful when they belong
to the same edition, but they do not replace the required game data.

Suggested layout:

```
~/.firestaff/data/
  dm1/
  csb/
  dm2/
  nexus/
  theron/
```

Use the launcher setting or `--data-dir` to select another root, then inspect
what was recognised:

```bash
firestaff --scan-data
firestaff --data-dir /path/to/games --scan-data
```

See [game-data setup](docs/DATA_SETUP.md) for the accepted media and the role
of optional files for each game.

## Running Firestaff

Build from source when a suitable package is not available:

```bash
git clone https://github.com/yeager/firestaff.git
cd firestaff
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/firestaff --scan-data
```

Firestaff requires CMake, a C11 compiler and SDL3. On macOS, SDL3 is available
through Homebrew:

```bash
brew install sdl3
```

Useful command-line options:

```text
firestaff --game <dm1|csb|dm2|nexus|theron>
          --data-dir <path>
          --scan-data
          --fullscreen
          --scale-mode <n>
          --version
```

Run the local test suite with:

```bash
ctest --test-dir build --output-on-failure
```

Some tests need original game data and skip when that corpus is not present.

## How the project is built

The launcher selects a game and its verified data. The game layer then owns
rendering, input and runtime state, while the data layer reads the original
files and models the dungeon.

```text
Launcher
  └─ Game runtime
       └─ Dungeon and data layer
            └─ Original game files supplied by the player
```

Gameplay work is checked against primary references. DM1 and CSB use
[ReDMCSB](http://dmweb.free.fr/Stuff/ReDMCSB_WIP20210206.7z), with CSBWin and
documented original formats as additional references. DM2 uses skproject;
Nexus and Theron's Quest use their respective platform analysis and original
media.

The [documentation index](docs/DOCUMENTATION_INDEX.md) links the user guides,
data notes and technical references. The [project status](docs/PROJECT_STATUS.md)
is the place to check the current boundary before relying on a development
route.

## Legal

Firestaff is a clean-room engine reimplementation. You need game files from
copies you legally own; the repository contains no copyrighted game data.

Dungeon Master, Chaos Strikes Back and Dungeon Master II are trademarks of FTL
Games. DM Nexus is a trademark of Victor Interactive Software. Theron's Quest
is a trademark of Working Designs and Victor Interactive Software.

## License

MIT. See [LICENSE](LICENSE).
