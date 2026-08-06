# Firestaff project status

This is the short, user-facing status summary for the whole project. Detailed
source audits and capture receipts remain in the linked game documentation.

| Game | Current status | What is verified | Main open boundary |
|---|---|---|---|
| Dungeon Master 1 | Playable/source-locked V1 | PC 3.4 runtime, viewport, HUD, input, combat, saves and original-data gates | Continued visual capture and V2 polish |
| Chaos Strikes Back | Active hardening | Source-locked engine slices, dungeon model, mechanics, startup and utility/import paths | End-to-end real-data runtime proof |
| Dungeon Master II: Skullkeep | Active hardening | Boot/profile, GDAT utilities, V2 presentation, lighting, HUD, movement and controller slices | V1 dungeon/render/mechanics parity with real assets |
| DM Nexus | Active real-data bring-up | Saturn DMDF/DGN data, world/render/save/mechanics slices and V2 presentation | Positive real-asset runtime handoff and full playability |
| Theron's Quest | Active real-media bring-up | JP/US Track 02 identity, parser, rendering, mechanics, progression, save/load and capture instrumentation | Game-owned Track 02 dungeon handoff, object/level records and real bitmap/palette bindings |

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
