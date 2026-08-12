# Firestaff project status

**Last reviewed: 2026-08-12.** This page is the concise status source for the
README and wiki. The full documentation map is in
[`DOCUMENTATION_INDEX.md`](DOCUMENTATION_INDEX.md).

This is the short, user-facing status summary for the whole project. Detailed
source audits and capture receipts remain in the linked game documentation.

| Game | Current status | What is verified | Main open boundary |
|---|---|---|---|
| Dungeon Master 1 | Playable PC V1 plus bounded Atari route | PC DOS 3.4 runtime, viewport, HUD, input, combat, saves and original-data gates; Atari ST launch/data path | Broader original-vs-Firestaff capture, V2 material, Amiga/FM Towns runtime proof and the C13 save corpus |
| Chaos Strikes Back | Verified native routes, not complete campaign parity | Amiga default; Atari ST legacy-save route; FM Towns English/Japanese startup, Utility Disk and input paths | Wider campaign/save/HUD/viewport parity and Extended Features/DSA corpus |
| Dungeon Master II: Skullkeep | Two English Macintosh routes verified to bounded New Game | DOS/FM Towns runtime slices; large Mac retail and small First Chapter ZIPs read in RAM with separate media, movie, sound, MIDI, input and New Game gates | Complete V1 parity; Mac GAME_LOAD/Resume and native dynamic pointer/drag ownership |
| DM Nexus | Bounded Saturn real-data phase launch | Authentic Japanese Saturn DM.BIN/Track 1, DGN/DMDF/MNS/PRS3 parsing and phase-launch gate | Full gameplay, material semantics, event/audio playback and public frame capture |
| Theron's Quest | Real-media data/parser bring-up | Japanese/US Track 02 identity, parser, level framing, mechanics and capture instrumentation | Game-owned dungeon handoff, saves, bitmap/palette binding and positive gameplay capture |

The dated [preservation status](PRESERVATION_STATUS_2026-08-11.md) records
the current source, format and real-media boundary for every game.

## CSB boundary in brief

CSBWin is retained as a source/disassembly reference for engine and format
research. It is not game data, not a Firestaff PC route, and cannot make CSB
into a DOS title: CSB has no original DOS release and `--platform pc` remains
closed. PC-9801 and X68000 are also unsupported CSB platforms.

## Shared presentation priority

Across all five games, presentation work is sequenced in the same order:

1. **Startup** — authentic media admission, title/intro route and launch gate.
2. **Menu** — source-bound navigation, options and game selection.
3. **HUD** — real bitmap, palette, font and layout ownership.
4. **Viewport** — source-backed map, object, creature, lighting and camera output.

An earlier stage must be stable before a later stage is described as complete.
This keeps a working launcher or synthetic HUD fixture from being mistaken for
full original-data gameplay.

## Evidence rules

“Verified” means that the relevant source or real-data receipt exists and its
focused checks pass. A synthetic fixture can prove a parser or state-machine
contract, but it cannot prove original-media parity. Screenshots in public
documentation must be real runtime captures; generated or fallback art is not
promoted as game evidence.

## Build and CI

The canonical local build is documented in [`DATA_SETUP.md`](DATA_SETUP.md)
and the CI checks in [`CI.md`](CI.md). GitHub Actions runs strict warnings,
asset hygiene, native builds on three operating systems, headless probes and a
cross-platform determinism comparison. Rapid pushes to `main` cancel obsolete
runs, so status must always be read from the newest commit.

The [Firestaff wiki](https://github.com/yeager/firestaff/wiki) expands this
summary with per-game format, hardware and reverse-engineering notes. The
checked-in wiki source is under [`docs/wiki/`](wiki/).
