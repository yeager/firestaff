# Firestaff project status

**Last reviewed: 2026-08-25.** This page is the concise status source for the
README and wiki. The full documentation map is in
[`DOCUMENTATION_INDEX.md`](DOCUMENTATION_INDEX.md).

This is the short, user-facing status summary for the whole project. Detailed
source audits and capture receipts remain in the linked game documentation.

| Game | Current status | What is verified | Main open boundary |
|---|---|---|---|
| Dungeon Master 1 | Playable/source-locked V1; native macOS host smoke verified | PC 3.4 runtime, viewport, HUD, input, combat, saves, Apple Silicon macOS boot and original-data gates | C13 save corpus, broader original-vs-Firestaff capture and V2 material; the unreleased historical Macintosh prototype has no local authenticated data |
| Chaos Strikes Back | Verified bounded real-media routes; active hardening | Atari STX title/runtime/start-menu CLI, Amiga and FM Towns bounded native routes | Explicit-STX M12 selection, DSA/save corpus, wider HUD/viewport and pixel evidence |
| Dungeon Master II: Skullkeep | Verified bounded real-media routes; active hardening | M12 source-owner selection, FM Towns M11 title/gameplay corpus, source-owned viewport/HUD and focused mechanics | Full record/save ownership and broader source UI/audio/mechanics parity across editions |
| DM Nexus | Active real-data bring-up | Saturn DMDF/DGN data, world/render/save/mechanics slices and V2 presentation | Saturn runtime/frame capture, material semantics, event/audio playback and full playability |
| Theron's Quest | Verified JP startup route; active real-media bring-up | JP Rev 1 title → stage → Soul Room, JP/US Track 02 identity, parser and level framing | Full Track 02 handoff, save body semantics, bitmap/palette binding and gameplay capture |

## Completion reporting

Firestaff does not assign invented percentages to a game, version or platform.
Completion is reported by the strongest evidence-backed status in the platform
matrix: **Playable**, **Verified route**, **Data path**, **Preservation**, or
**Unsupported**. A verified boot probe is therefore not reported as completed
gameplay, and a parser or synthetic fixture never raises a row above its
real-media evidence.

| Game | Completion status | Platform source of truth |
|---|---|---|
| Dungeon Master 1 | Playable on PC DOS 3.4; other editions remain separately scoped. | [DM1 platform rows](PLATFORM_STATUS.md#dungeon-master) |
| Chaos Strikes Back | Verified bounded routes on Atari ST, Amiga and FM Towns; no original DOS/PC release. | [CSB platform rows](PLATFORM_STATUS.md#chaos-strikes-back) |
| Dungeon Master II: Skullkeep | Verified bounded routes on DOS, FM Towns and the listed Macintosh editions. | [DM2 platform rows](PLATFORM_STATUS.md#dungeon-master-ii-the-legend-of-skullkeep) |
| DM Nexus | Verified bounded Japanese Saturn route, not production gameplay. The measured implementation coverage is documented separately. | [Nexus platform row](PLATFORM_STATUS.md#dm-nexus), [Nexus completion](NEXUS_COMPLETION.md) |
| Theron's Quest | JP and US Track 02 are data paths; no end-to-end gameplay claim. | [Theron platform rows](PLATFORM_STATUS.md#therons-quest) |

Nexus is the sole exception with an explicitly defined numeric measurement:
44.0% implementation coverage across its six measured domains, 33.1% for its
weighted startup-to-HUD chain, and 0% production grade. Those values describe
only the documented Saturn capture model and must not be averaged with the
status labels above.

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
