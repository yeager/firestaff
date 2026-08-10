# Firestaff

Firestaff is a clean-room engine for the Dungeon Master games. It reads the
original files you own, identifies each edition by its content hash and keeps
that data separate from the program.

Dungeon Master for PC DOS 3.4 is the current playable route. Chaos Strikes
Back now starts from its verified native Amiga editions by default; its full
campaign, save and presentation parity are still under active development.
Dungeon Master II, DM Nexus and Theron's Quest are also active development
routes, not finished releases.

[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## Current state

Dungeon Master for PC DOS 3.4 is the most complete route: startup, menus,
dungeon view, HUD, controls, combat and save handling use original data.

The other games are active development routes. Firestaff detects their real
media and exposes only paths with a verified handoff; it never borrows data
from another edition to fill a gap. The detailed status is kept in
[project status](docs/PROJECT_STATUS.md). The published documentation is
available at [yeager.github.io/firestaff](https://yeager.github.io/firestaff/).

| Game | Current scope |
|---|---|
| Dungeon Master | Playable PC DOS 3.4 route; further parity work continues. |
| Chaos Strikes Back | Native Amiga is the default route when verified media is available. Atari ST and FM Towns have their own native data and startup paths. Campaign parity is still being completed. |
| Dungeon Master II: Skullkeep | Engine and data work in progress. |
| DM Nexus | Saturn real-data bring-up in progress. |
| Theron's Quest | PC Engine real-media bring-up in progress. |

### Theron's Quest: original reference capture

This is a real in-game capture from the original US PC Engine CD release in
Mednafen. It is included as a visual reference for the Theron bring-up; it is
not presented as proof that Firestaff has reached full rendering or gameplay
parity.

![Original Theron's Quest US dungeon reference capture](verification-screens/theron-quest-us-dungeon-mednafen.png)

## Chaos Strikes Back editions

Firestaff recognises original CSB editions by hash rather than by their folder
names. The scanner currently covers the following families when the required
matching data is present:

| Original family | What Firestaff does with it today |
|---|---|
| Amiga 3.1 and 3.5 | Default CSB route when a verified native program handoff is available. The native startup and presentation routes are being hardened against the original editions. |
| Atari ST 2.0 and 2.1 | Native Atari media is recognised and enters a dedicated Atari startup route. |
| FM Towns English and Japanese | Native CD installations are recognised and use their version-specific Towns packages for the available title, Game and Utility routes. |
| PC DOS 3.4 | Recognised source-reference edition. It remains useful for format and runtime comparison, but is not selected ahead of verified native Amiga CSB media. |

Recognition is deliberately separate from a playability claim. A recognised
edition has passed the data gate; it does not imply that every screen, save
format or gameplay path has reached parity. Firestaff keeps each edition on
its own data path and never borrows files from another release to make a route
appear to work.

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

Original BIOS/firmware and game media are never bundled with Firestaff or
stored in this repository. Supply your own legally obtained files through the
local data directory; repository CI enforces this boundary.

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

The [documentation index](https://yeager.github.io/firestaff/DOCUMENTATION_INDEX.md)
links the user guides,
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
