# Firestaff documentation

This is the documentation map for the complete Firestaff project. It covers
all five supported games, the shared launcher and engine layers, data setup,
CI, source-lock evidence and release packaging.

## Current project boundary

The public status is intentionally conservative. A parser, fixture or
source-lock test proves a narrow contract; it does not by itself prove
original-media playability or pixel parity.

| Game | Verified today | Active boundary |
|---|---|---|
| Dungeon Master (DM1) | Playable PC 3.4 V1 runtime with source-locked startup, HUD, viewport, combat, saves and original-data gates | Continued original-route captures and V2 visual polish |
| Chaos Strikes Back (CSB) | Source-locked engine slices, startup/entrance, dungeon model, DSA, mechanics, utility/import and rendering contracts | End-to-end real-data runtime proof |
| Dungeon Master II: Skullkeep | Boot/profile, GDAT utilities, V2 presentation, lighting, HUD, movement, controller and real-data slices | V1 dungeon/render/mechanics parity with real assets |
| DM Nexus | Saturn DMDF/DGN, world, save, mechanics, V2 presentation, MNS/SAL/MAP and bounded PRS3 evidence | Positive real-asset runtime handoff, visible material rendering and full playability |
| Theron's Quest | JP/US Track 02 identity, startup records, descriptor/level framing, parser, mechanics, progression, save boundary and capture instrumentation | Game-owned Track 02 dungeon handoff, object/level semantics, bitmap/palette binding and app capture |

Across every game, work is ordered as **startup → menu → HUD → viewport**.
Later presentation claims remain blocked when an earlier source or real-data
handoff is not proven.

## Start here

- [README](../README.md) — user-facing overview, supported platforms and quick start
- [Project status](PROJECT_STATUS.md) — short cross-game status matrix
- [Game data setup](DATA_SETUP.md) — legal data intake and scanner behavior
- [CI guide](CI.md) — GitHub Actions checks and local reproduction
- [Verified hashes](VERIFIED_HASHES.md) — canonical data identities
- [Gap list](FIRESTAFF_GAP_LIST.md) — active technical boundaries
- [DMWeb and Greatstone references](DMWEB_REFERENCE.md) — external provenance index
- [Theron real-data inventory](THERON_REALDATA_INVENTORY.md) — Track 02/19 media identities and placeholder boundaries

## Wiki map

The checked-in pages under [`docs/wiki/`](wiki/) are the source for the GitHub
wiki and are copied by `scripts/sync_wiki.sh` during release work.

### Per-game pages

- **DM1:** [technical reference](wiki/DM1-Technical-Reference.md), [PC34 internals](wiki/DM1-PC34-Internals.md), [reverse engineering](wiki/DM1-Reverse-Engineering.md)
- **CSB:** [technical reference](wiki/CSB-Technical-Reference.md), [DSA and saves](wiki/CSB-DSA-and-Save-Internals.md), [reverse engineering](wiki/CSB-Reverse-Engineering.md)
- **DM2:** [technical reference](wiki/DM2-Technical-Reference.md), [GDAT internals](wiki/DM2-GDAT-Internals.md), [reverse engineering](wiki/DM2-Reverse-Engineering.md)
- **Nexus:** [technical reference](wiki/Nexus-Technical-Reference.md), [DGN/PRS3](wiki/Nexus-DGN-and-PRS3-Internals.md), [SAL/MAP](wiki/Nexus-SAL-MAP-Internals.md), [reverse engineering](wiki/Nexus-Reverse-Engineering.md)
- **Theron:** [technical reference](wiki/Therons-Quest-Technical-Reference.md), [Track 02 internals](wiki/Therons-Quest-Track02-Internals.md), [reverse engineering](wiki/Therons-Quest-Reverse-Engineering.md)

### Shared pages

- [Wiki home](wiki/Home.md)
- [Reverse-engineering index](wiki/Reverse-Engineering-Index.md)
- [Game data](wiki/Game-Data.md)
- [Architecture](wiki/Architecture-Overview.md)
- [Building and installing](wiki/Building-and-Installing.md)
- [Parity evidence](wiki/Parity-Evidence.md)
- [Platform guides](wiki/Platform-macOS.md)
- [Release process](wiki/Release-Process.md)

## Evidence vocabulary

- **Source-locked:** behavior is anchored to a reference source or disassembly and guarded by a focused test.
- **Real-data verified:** the tested asset is identified by its canonical hash and the relevant parser/loader contract passes.
- **Runtime proven:** the real-data path reaches the claimed runtime milestone.
- **Playable:** the public route has end-to-end original-data runtime proof; this is currently reserved for DM1 V1.
- **Fixture-only:** useful for a narrow parser or state-machine contract, but not evidence for original-media rendering.

## Rebuilding the documentation

Documentation changes are ordinary repository changes. Before publishing a
release, run the focused tests for the affected game, check Markdown links and
run the build/CI commands in [CI.md](CI.md). Do not commit original game data,
emulator saves, private capture logs or generated placeholder art.

**Last reviewed:** 2026-08-06.
