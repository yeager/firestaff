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
| Save corpus | Header, raw dungeon prefix, fixed `DM2_GAME_LOAD` SUPPRESS sections, DB-pool records and transactional rejection | The real PC-DOS corpus is inspected without unpacking or modifying it. The eight supplied DOS save/backup members resume through the menu into an accepted M11 frame; original writing and broader record-link/possession parity remain open. |
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

### Native CLI start and first-movement probes

These skip-safe CTest entries use the selected original archive directly in
memory.  Each proves a source-owned startup/menu sequence, a loaded runtime
and the first real movement input; none claims broader save, combat or GUI
parity.

| Platform/media | CTest name | Authenticated GDAT MD5 |
| --- | --- | --- |
| DOS EN | `dm2_v1_dos_native_cli_boot` | `25247ede4dabb6a71e5dabdfbcd5907d` |
| DOS FR | `dm2_v1_dos_fr_native_cli_boot` | `b4d733576ea60c41737f79f212faf528` |
| Amiga EN installer ZIP | `dm2_v1_amiga_native_cli_boot` | `1c940ea95703eaea0ecdf84d17e954b9` |
| Macintosh EN retail ZIP | `dm2_v1_mac_native_cli_boot` | `5cab25f6b975957eae4a203174e7f2a6` |
| FM Towns HME-242 ZIP | `dm2_v1_fmtowns_native_cli_boot` | `027ff3b8ddc2c4c4cdda7ada0b0bc46c` |

Every probe enters through Firestaff's start menu (`--menu`) before it reaches
the original startup path. The Macintosh probe dismisses its retained title
movie, dispatches the original New Game event and selects a source-owned
viewport mirror. The FM Towns probe preserves its AUTOEXEC/SWOOSH/TITLE/SKULL
handoff before the source-space mirror selection. All routes refuse unrelated
PC coordinates or extracted media.

`dm2_v1_dos_sksave_archive_menu_resume` adds the corresponding resume route:
the DOS menu receives `archive.zip::data/sksave1.dat`, retains it in RAM, and
must publish its authenticated map 11 / party 15,10,2 session. It does not
write or unpack the original archive.

`dm2_v1_dos_sksave_archive_menu_resume_matrix` applies that same read-only
menu route to all four supplied DOS slots and their backups. It asserts the
distinct original map/position receipts for all eight members, plus an
accepted M11 frame backed by real GDAT material with zero fallback draws.
That prevents a fallback to either one synthesized default save state or a
black/unowned runtime frame.

The companion CTest `test_dm2_v1_dos_sksave_archive_resume_real_media`
commits `sksave1.dat` through the same archive reader and verifies the first
source-runtime turn directly. This isolates live GAME_LOAD ownership from
launcher-script timing; it does not claim a full interactive save campaign.

## Remaining blockers to playable parity

The following are deliberately unavailable rather than replaced with
placeholders or synthetic state:

- Original SKSAVE writing, exhaustive record-link restoration, possessions
  and live allocation parity beyond the verified DOS corpus are not complete.
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
