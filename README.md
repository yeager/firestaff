# Firestaff

Firestaff is a clean-room engine for the Dungeon Master games. It reads the
original files you own, identifies each edition by its content hash and keeps
that data separate from the program.

Firestaff can start verified original media for Dungeon Master, Chaos Strikes
Back and Dungeon Master II: Skullkeep across their supported platforms.
Coverage is deliberately conservative: a route is available only when the
selected edition has a verified native handoff. Full campaign, save and visual
parity remain active work, especially for Chaos Strikes Back, Nexus and
Theron's Quest.

[![CI](https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg)](https://github.com/yeager/firestaff/actions/workflows/verify.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="360">
</p>

## Current status

The status below was reviewed on 2026-09-07. It reports what has been exercised
with real media, not a claim of complete game parity.

Firestaff detects real media and exposes only paths with a verified handoff; it
never borrows data from another edition to fill a gap. The detailed status is kept in
[project status](docs/PROJECT_STATUS.md). The published documentation is
available at [yeager.github.io/firestaff](https://yeager.github.io/firestaff/).
The [preservation status](docs/PRESERVATION_STATUS_2026-08-11.md), reviewed
2026-08-12,
separates source/disassembly evidence, real-media receipts and open routes.

| Game | Current scope |
|---|---|
| Dungeon Master | PC DOS, Atari ST, Amiga and FM Towns startup and selected dungeon routes have real-media coverage. Further gameplay and visual parity work continues. |
| Chaos Strikes Back | Amiga, Atari ST and FM Towns startup routes have real-media coverage. Campaign, saves and presentation parity are still being completed. |
| Dungeon Master II: Skullkeep | DOS, Amiga, FM Towns and Macintosh have real-media startup and selected runtime coverage. Advanced parity, saves and some combat/UI behavior continue. |
| DM Nexus | Saturn disc parsing and native title-resource loading work; the public title is correctly blocked pending authenticated display-state captures. |
| Theron's Quest | PC Engine/TurboGrafx real-media startup and initial dungeon parsing work; later presentation and level-transition evidence is still required. |

### Dungeon Master II: Skullkeep

DM2 is playable in Firestaff from four authenticated source families:

| Edition | Accepted source data | Verified runtime scope |
|---|---|---|
| DOSBox / PC English | `GRAPHICS.DAT` + `DUNGEON.DAT`; DOSBox saves in `Downloads/dm2` are optional resume data | New Game, active runtime, movement, pits, stairs, level transitions, creatures and spell handoff |
| Amiga English | Original installer archive, read and verified in memory | New Game, active big-endian runtime, clipped source CHARSHEET inventory, movement, pits, stairs, level transitions and creatures |
| FM Towns Japanese | Original HME-242 ZIP/disc image; non-Japanese text uses the built-in GDAT-keyed l10n bridge | Title sequence, New Game, inventory, movement, level transitions and creatures |
| Macintosh English | Authentic retail ZIP/HFS media | New Game, active big-endian runtime, movement, stairs, level transitions and combat/creature handoff |

The shared DM2 data root may contain all four editions. Firestaff resolves a
selected version to its own source owner: the DOS `data` tree or symlink,
Amiga installer, FM Towns disc archive, or Mac archive. Original archives are
kept intact and archive members are read into bounded memory; Firestaff does
not use a sibling edition as a fallback.

### Language selection

Firestaff offers 20 interface languages: English, Swedish, German, French,
Spanish, Italian, Portuguese, Dutch, Polish, Czech, Russian, Japanese,
Korean, Simplified Chinese, Danish, Norwegian, Finnish, Hungarian, Turkish
and Indonesian. **Auto** uses the system language when it is supported and
otherwise uses English. A language chosen in the start menu or with
`--lang <code>` takes precedence over Auto.

For the Japanese FM Towns edition of DM2, Firestaff keeps the original Towns
disc as the only game-data owner. A built-in, GDAT-keyed bridge maps the
disc's authenticated text records to canonical English gettext entries, so no
PC-English `GRAPHICS.DAT` is required at runtime. The selected catalog then
applies Swedish or another supported language. Swedish has a complete DM2
catalog; other language catalogs remain work in progress. Any untranslated
entry safely shows its canonical English source text—never invented text or a
different edition's Japanese fallback. See [translation status](po/README.md)
for exact per-language coverage.

Focused real-media checks and their current boundaries are documented in
[TODO-dm2.md](TODO-dm2.md), [DONE-dm2.md](DONE-dm2.md),
[DM2 platform variants](docs/dm2_variants_platform.md)
and the [DM2 FM Towns wiki guide](docs/wiki/DM2-FMTowns-Guide.md).

Theron's Quest uses ordinary desktop controls in Firestaff: Up/W moves
forward, Down/S moves backward, and Left/A and Right/D turn while held.
Keypad 8/2/4/6 provides the same four directions. Left and right mouse
buttons are Button I and Button II; mouse motion only moves the normal pointer
and never changes the selected object or jumps between controls. On touch
screens, a short touch is Button I and a long touch is Button II.

### Theron's Quest runtime status

README screenshots are Firestaff-rendered screenshots only. Original-media
emulator captures are kept out of the public README and are not presented as
Firestaff output. Theron's Quest remains in source-bound runtime bring-up;
see the [capture handoff record](docs/source-lock/theron-authentic-track02-handoff-2026-08-08.md)
for the non-visual media and trace evidence.

## Chaos Strikes Back editions

Firestaff recognises original CSB editions by hash rather than by their folder
names. The scanner currently covers the following families when the required
matching data is present:

| Original family | What Firestaff does with it today |
|---|---|
| Amiga 3.1 and 3.5 | Default CSB route when a verified native program handoff is available. Native startup, entrance, supported HUD and viewport material use the Amiga data path. |
| Atari ST 2.0 and 2.1 | Native media uses its own animation, runtime, HUD and supported viewport-material routes. |
| FM Towns English and Japanese | Native CD installations use their version-specific Towns packages for the supported title, Game and Utility routes. |
| PC-9801 Japanese 3.1 | Not supported. The media is retained only as preservation reference and cannot select a data, startup, gameplay or input route. |
| X68000 Japanese 3.1 | Not supported. The media is retained only as preservation reference and does not select a data, startup or gameplay route. |

Recognition is deliberately separate from a playability claim. A recognised
edition has passed the data gate; it does not imply that every screen, save
format or gameplay path has reached parity. Firestaff keeps each edition on
its own data path and never borrows files from another release to make a route
appear to work.

### CSBWin legacy saves

A complete original-named legacy CSBWin slot can resume with matching Atari
ST 2.0/2.1 `GRAPHICS.DAT` and `DUNGEON.DAT`, from both the launcher and CLI:

```bash
firestaff --game csb --data-dir /path/to/CSB --save /path/to/CSBGAME2.DAT
```

The loader validates the full save body and source provenance before starting.
It does not treat an arbitrary 512-byte header, renamed copy, compact roster,
or `DMSAVE.*` file as a CSB resume. Extended Features/DSA saves remain
fail-closed pending authenticated real-save coverage; its behavior is
source-locked against CSBWin reference code and does not create a CSBWin game
route.

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
of optional files for each game. The [game-data format reference](docs/GAME_DATA_FORMATS.md)
explains the verified containers, record families and save boundaries.

Firestaff never requires, searches for, reads, or bundles a BIOS, firmware,
System Card, or external emulator. The only runtime input is your legally
obtained game data in the local data directory; repository CI enforces this
boundary.

The reproducible source dependency inventory is available as
[`sbom/firestaff.spdx.json`](sbom/firestaff.spdx.json) (SPDX 2.3). It excludes
game media and every user-local input.

## Included tools

Firestaff also ships desktop tools for working with files you own. They are
optional and never run while playing a game.

| Tool | Purpose | Documentation |
|---|---|---|
| Firestaff Artpack Studio | Creates and validates Modern-mode artpacks without modifying original game media. | [Artpack Studio guide](docs/artpack_studio.md) |
| Firestaff Dungeon Studio | Views and edits supported dungeon data, with a built-in screenshot option for documentation and review. | [Dungeon Studio source](scripts/firestaff_dungeon_studio.py) |
| Firestaff Savegame Editor | Inspects and edits supported save files; always keep a backup of an original save. | [Savegame Editor source](scripts/firestaff_savegame_editor.py) |

The desktop bundles build translations from their `.po` source catalogs during
packaging. Generated `.mo` files are not stored in the source tree.

### Platform status at a glance

| Game | Playable | Verified runtime routes | Data/preservation only | Unsupported |
|---|---|---|---|---|
| DM1 | — | PC DOS, Atari ST, Amiga and FM Towns startup and selected dungeon routes | PC-9801 preservation | X68000 |
| CSB | — | Atari ST, Amiga and FM Towns title/start-menu routes | — | PC-9801, X68000 |
| DM2 | — | DOS, Amiga, FM Towns and Macintosh startup plus selected runtime routes | Mac JP/FR preservation | X68000 |
| Nexus | — | Saturn disc/resource parsing; title display remains blocked pending authenticated captures | Saturn demo/fan translations | — |
| Theron's Quest | — | PC Engine/TurboGrafx US ZIP and Japanese CUE/Track 02 startup plus initial dungeon parsing | Later gameplay, saves and presentation remain evidence-gated | — |

This table is a summary. Use [Platform status](docs/PLATFORM_STATUS.md) for
the exact feature boundary and [Project status](docs/PROJECT_STATUS.md) for
cross-game evidence rules.

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
          --platform <auto|pc|amiga|atari-st|fm-towns|pce|saturn>
          --dm1-fmtowns-ja
          --csb-fmtowns-ja
          --csb-utility-disk
          --scan-data
          --fullscreen
          --scale-mode <n>
          --version
```

Nexus remains deliberately fail-closed in Firestaff's native runtime until a
real Saturn title/display-consumer capture exists.  Firestaff never delegates
Nexus startup or gameplay to Mednafen (or another emulator); emulator tooling
is used only outside the product to obtain and validate capture evidence.

`--csb-fmtowns-ja` explicitly selects the hash-verified Japanese FM Towns
package and fails if that original package is not present; it never guesses
from the host language or substitutes the English package.

`--dm1-fmtowns-ja` explicitly selects the hash-verified Japanese FM Towns
edition and fails closed if its original members are unavailable; the default
FM Towns selection remains the English edition.
### Theron's Quest (PC Engine CD)

Place the original US Track 02 BIN in `.firestaff/data/theron/` (or pass a
data root that contains `theron/TQUS02.bin`) and start it normally:

```bash
./build/firestaff --game theron --data-dir "$HOME/.firestaff/data"
```

The title accepts Enter, followed by the stage and Soul Room selections. The
Japanese Rev 1 CUE then reaches the bounded native Akutuba runtime through
hash-verified Track 02 records. For a headless, reproducible CLI receipt, use:

```bash
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./build/firestaff \
  --game theron --data-dir "$HOME/.firestaff/data/theron" --boot-probe \
  --script 'enter,enter,action' \
  --boot-probe-expect-phase theron-runtime --boot-probe-expect-runtime \
  --boot-probe-expect-level-loaded 1 --boot-probe-expect-party 1,0,0 \
  --boot-probe-expect-startup-active 0
```

This confirms the source-backed title → stage → Soul Room → initial runtime
handoff, not broad PC Engine gameplay parity. Uncaptured later-level/object
publication, creature AI, combat, generator, sound-effect and text-control
semantics remain unavailable rather than being replaced with host behavior.
`--csb-utility-disk` opens the separately preserved FM Towns Utility Disk
after normal verified CSB startup; it implies `--game csb --platform fm-towns`
and fails closed if that package is unavailable. The start menu also has a
dedicated **CSB Utility Disk (FM Towns)** entry. This is distinct from the
Atari Hint Oracle (`--csb-hint-oracle`) and never substitutes its data or UI.
The Hint Oracle needs `--data-dir <root>` containing its matching verified
files; those files may be loose or inside a supported archive. The Japanese
Utility Disk chooser additionally requires the user's authorised FM Towns
font ROM for original Shift-JIS glyphs. Firestaff never substitutes a system
font or installs the ROM into game data.

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
