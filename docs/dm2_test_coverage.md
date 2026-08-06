# DM2 V1 — coverage and remaining parity work

## Current scope

This document describes the current DM2 codebase. It replaces the old
bootstrap note which incorrectly described DM2 as eleven untested stubs.
The configured CTest manifest currently contains 310 DM2-named tests. They
cover source-bound parsers, material receipts and narrowly scoped runtime
handoffs. That is not evidence that DM2 is fully playable.

Original game files remain user-supplied. Tests that need them are skip-safe
when no corpus is configured, and use the mounted PC-DOS data when
`FIRESTAFF_DM2_DATA_DIR` or `FIRESTAFF_DM2_SKSAVE_CORPUS` is set.

## Covered source boundaries

| Area | Evidence | Honest status |
| --- | --- | --- |
| Boot, title and menu | Original `TITLE` and `INTERFACE_GENERAL` GDAT records, source pointer rectangles and palette receipts | Real-data startup is covered. Full original keyboard-event translation remains open. |
| GDAT | PC-DOS, Mac, Amiga, FM Towns, PC-9821 and Mega-CD parser boundaries; raw image, palette, animation and command plans | Decoding and provenance are covered; every presentation family still needs a live runtime owner. |
| Dungeon and G1 | Map header, first-map, record-base, tile links, scene/material, static object and local-palette gates | Real input is decoded and bounded. Complete live dungeon rendering and mechanics remain open. |
| HUD and viewport | GDAT HUD-command receipts, portraits, item/creature local palettes, M11 material handoffs | The active path is source-owned and rejects generated overlays. It is not yet full original GUI/viewport parity. |
| Save corpus | Header, raw dungeon prefix, fixed `DM2_GAME_LOAD` SUPPRESS sections, DB-pool records and transactional rejection | The real PC-DOS corpus is inspected without unpacking or modifying it. Resume and original writing remain blocked until the full record-link/possession graph is live. |
| Sound | GDAT PCM receipt/decoder, queue ordering, positional attenuation and SDL backend | Sound data is real when a verified entry exists. PC HMP music stays silent until its original timing/decoder path is implemented. |
| Mechanics | Source-locked helper tests for movement, records, timers, creature, CCM, projectile, spells, doors, shops and actuators | Most are bounded components or test-only transcripts until connected to the real loaded world. |
| Outdoor scenes | G1/GDAT palette and environment-material gates | Missing source-owned environment timer and image selection keep outdoor drawing fail-closed. |

## Real-data regression entry points

The following focused tests are useful when the PC-DOS corpus is available:

```sh
FIRESTAFF_DM2_DATA_DIR=/path/to/dm2/data \
  ./build/test_dm2_v1_m11_startup_profile_gate

FIRESTAFF_DM2_SKSAVE_CORPUS=/path/to/dm2/data \
  ./build/test_dm2_v1_save_load_real_data

FIRESTAFF_DM2_DATA_DIR=/path/to/dm2/data \
  ./build/test_dm2_v1_sound_gdat_real_data
```

`test_dm2_v1_save_load_real_data` currently checks all eight supplied
`sksave0..3.dat/.bak` files. It verifies the 42-byte original header, raw
dungeon boundaries, the shared fixed SUPPRESS stream and first/final records
of each non-empty DB pool. It deliberately does not publish a playable
session.

## Remaining blockers to playable parity

The following are deliberately unavailable rather than replaced with
placeholders or synthetic state:

- Original SKSAVE resume and writing: `DM2_READ_SKSAVE_DUNGEON`, record-link
  restoration, possessions, live allocation and post-load ownership are not
  complete.
- The complete source menu/HUD event loop, including keyboard/controller
  translation and live champion GUI state.
- Full G1 dungeon/viewport composition, object chains, door state, lighting
  and outdoor environment selection in the M11 runtime.
- HMP playback timing and a source-proven music backend for PC-DOS music.
- End-to-end gameplay proof across timers, movement, combat, triggers and
  save/load with the same authenticated world state.

## Rules for future coverage

- Prefer original bytes and source references from SKProject, ReDMCSB,
  Greatstone and DMWeb.
- Keep tests data-free only for isolated algorithms. Do not turn fixtures,
  inferred labels, generated art or host state into player-facing DM2 data.
- Treat a successful parse, receipt or component test as a local proof, not a
  claim of playable parity.
- When a source owner is missing, leave the production path unavailable and
  record the reason in `TODO.md`.
