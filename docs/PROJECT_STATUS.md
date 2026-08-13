# Firestaff project status

**Last reviewed: 2026-08-13.** This page is the concise status source for the
README and wiki. The full documentation map is in
[`DOCUMENTATION_INDEX.md`](DOCUMENTATION_INDEX.md).

This is the short, user-facing status summary for the whole project. Detailed
source audits and capture receipts remain in the linked game documentation.

| Game | Current status | What is verified | Main open boundary |
|---|---|---|---|
| Dungeon Master 1 | Playable/source-locked V1; native macOS host smoke verified | PC 3.4 runtime, viewport, HUD, input, combat, saves, Apple Silicon macOS boot and original-data gates | C13 save corpus, broader original-vs-Firestaff capture and V2 material; the unreleased historical Macintosh prototype has no local authenticated data |
| Chaos Strikes Back | Active hardening | Source-locked engine slices, dungeon model, mechanics, startup and utility/import paths | DSA/save corpus, wider real-data runtime, HUD/viewport and pixel evidence |
| Dungeon Master II: Skullkeep | Playable across DOS, Amiga, FM Towns and Macintosh real media; active hardening continues | M12 source-owner selection, M11 New Game handoff, source-owned viewport/HUD, movement, pit/stairs/DB1, creatures and focused combat/spell routes | Native Amiga/FM Towns/Mac saves, positive DOS WIELD/death-drop fixture, broader source UI/audio/mechanics parity |
| DM Nexus | Active real-data bring-up | Saturn DMDF/DGN data, world/render/save/mechanics slices and V2 presentation | Saturn runtime/frame capture, material semantics, event/audio playback and full playability |
| Theron's Quest | Active real-media bring-up | JP/US Track 02 identity, parser, level framing, mechanics, progression and capture instrumentation | Full Track 02 handoff, save body semantics, bitmap/palette binding and JP capture |

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
