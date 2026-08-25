# Game and platform status

**Reviewed 2026-08-25.** This is the operational status matrix. It separates
media recognition, verified runtime slices and end-to-end playability. A row
marked as supported does not mean that every menu, save format or visual
parity gate is complete.

## Status terms

| Status | Meaning |
|---|---|
| **Playable** | Original data reaches normal gameplay through a maintained runtime gate. |
| **Verified route** | Authentic media is read and a bounded startup, New Game or gameplay route is tested. |
| **Data path** | The format or media is admitted and source-locked, but end-to-end gameplay is not proven. |
| **Preservation** | The edition is documented or retained as reference only. It cannot select a normal game route. |
| **Unsupported** | The platform is outside the game's Firestaff support matrix. |

## Dungeon Master

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC DOS 3.4 | **Playable** | V1 startup, menus, viewport, HUD, input, combat and saves use matching original data. | Broader original-vs-Firestaff capture and V2 finished material. |
| Atari ST | **Verified route** | Native Atari graphics/data path and launch admission are source-locked. | Complete gameplay parity and original-media capture. |
| Amiga | **Data path** | Amiga format and source references are documented. | Authenticated media, native runtime ownership and end-to-end proof. |
| FM Towns | **Data path** | CD layout and platform-specific input/audio boundaries are classified. | Authenticated runtime media, executable handoff and CD-audio gameplay proof. |
| PC-9801 | **Preservation** | Japanese floppy provenance is documented. | No Firestaff runtime route. |
| X68000 | **Unsupported** | Not part of the DM1 support matrix. | None planned in the current target. |

## Chaos Strikes Back

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Atari ST | **Verified route** | Native STX title, 50 Hz `ANIMATE.SCR`/`ANIMATE.DAT` title path, FTLCODE handoff, first runtime HUD/viewport frame and start-menu CLI route are tested against supplied campaign media. | Broader campaign and capture parity. |
| Amiga | **Verified route** | Verified native Amiga family is the default CSB route; startup and bounded runtime slices pass with original data. | Full campaign, save and visual parity. |
| FM Towns | **Verified route** | English/Japanese native packages have separate data, startup, Utility Disk and input paths. | Wider gameplay, Champion Editor and CD-audio parity. |
| DOS / PC | **No original release** | CSB has no DOS/PC edition. `--platform pc` is closed before media selection; CSBWin is source/disassembly evidence only. | No PC runtime route is planned from CSBWin; it must not be presented as DOS support. |
| PC-9801 | **Unsupported** | Preservation reference only. | None. |
| X68000 | **Unsupported** | Preservation reference only. | None. |

## Dungeon Master II: The Legend of Skullkeep

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| DOS | **Verified route** | Hash-verified PC data, GDAT/G1 loading, startup, runtime slices and sound tests. | Complete V1 renderer/mechanics parity and full SKSAVE ownership. |
| Amiga | **Verified route** | Original Amiga archive reaches title, New Game, bounded runtime and the clipped native CHARSHEET inventory frame through verified GDAT/RAW4 material. | Wider gameplay, save and full pixel/audio parity. |
| FM Towns | **Verified route** | Authentic Towns data and platform-specific startup, input, CD-audio and gameplay slices are tested. | Complete native runtime ownership and full parity. |
| Macintosh large retail | **Verified route** | Authentic English retail ZIP is read in RAM. HFS, big-endian dungeon data, title/movie/audio/MIDI resources, New Game and bounded wall input pass. | Complete Mac GAME_LOAD/Resume, native dynamic pointer/drag owner, CoreMIDI timing and full pixel/audio parity. No authentic Mac save is present. |
| Macintosh small First Chapter demo | **Verified route** | Authentic demo ZIP is read in RAM. Its StuffIt `DMFiles`, truncated dungeon, 16-entry roster, static startup, New Game and source-locked input pass independently. | The authoritative ZIP has no application fork, so dynamic Mac Control/Event ownership and Resume remain closed. |
| Macintosh Japanese/French | **Preservation** | Authentic media is retained or classified as preservation input. | Separate graphics/dungeon/runtime ownership and language-specific proof. |
| X68000 | **Unsupported** | Not part of the DM2 support matrix. | None planned in the current target. |

## DM Nexus

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| Sega Saturn Japanese | **Verified route** | Authentic Track 1/DM.BIN, CUE-declared CDDA Track 02–09 BIN ownership, DGN/DMDF/MNS/PRS3 parsing and bounded phase-launch tests. | Full gameplay, visible material semantics, event/audio playback and public capture parity. |
| Saturn demo or fan translations | **Preservation** | Classified separately from the canonical Japanese retail route. | Independent media and runtime proof. |

## Theron's Quest

| Platform | Status | Current scope | Open boundary |
|---|---|---|---|
| PC Engine/TurboGrafx-16 Japanese | **Verified route** | Authentic Rev 1 CUE reaches native title, stage, Soul Room and the source Akutuba runtime handoff (`party=1,0,0`); Track 02 identity and level framing are verified. | Broader game-owned runtime, bitmap/palette binding, saves and positive gameplay capture. |
| PC Engine/TurboGrafx-16 US | **Data path** | Authentic Track 02 identity and shared parser/runtime boundary. | US gameplay handoff and capture proof. |

## Data and preservation rules

Firestaff reads the original file or container in its supplied form. It may
use temporary in-memory decoding during development, but production does not
extract game data to a Firestaff-owned directory. A DOSBox save is never
treated as a Macintosh save, and a parser or synthetic fixture never upgrades
a row to playable. See [game-data setup](DATA_SETUP.md),
[preservation](wiki/Preservation.md) and [verified hashes](VERIFIED_HASHES.md).
