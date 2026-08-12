# Dungeon Master II: Skullkeep Technical Reference

> **Status reviewed 2026-08-12.** DM2 has verified DOS and FM Towns runtime
> slices. Both authentic English Macintosh editions boot from their original
> ZIPs in RAM and pass bounded New Game/media/input gates. Full V1 parity,
> Mac GAME_LOAD/Resume and native Mac pointer/drag ownership remain open.

## Scope

DM2 V1 follows skproject. It is independent from the DM1/CSB compatibility
path. DOS uses the paired PC `GRAPHICS.DAT` plus `DUNGEON.DAT` hashes; the
Towns and Macintosh routes keep their original platform containers, byte
order and resource ownership separate.

## Macintosh editions

Firestaff currently admits two English Macintosh editions independently:

| Edition | Verified | Still closed |
|---|---|---|
| Large retail | Authentic ZIP/BIN, HFS, big-endian dungeon, title/MooV, sound, MIDI, input, New Game and bounded wall interaction | Complete GAME_LOAD/Resume, native dynamic Control/Event pointer owner, CoreMIDI timing and full parity |
| Small First Chapter demo | Authentic ZIP, in-memory StuffIt `DMFiles`, truncated big-endian dungeon, 16-entry roster, static startup, input and New Game | The authoritative ZIP has no application fork; dynamic pointer/drag ownership and Resume cannot be claimed |

Production never extracts these game files to a Firestaff data directory and
never borrows the retail application fork for the demo. The DOSBox saves in
`Downloads/dm2` are DOS saves and are not valid evidence for Macintosh Resume.
The [platform status matrix](../PLATFORM_STATUS.md) is the current summary;
the [Mac source-lock note](../source-lock/dm2-mac-editor-preservation-note.md)
records the application/resource evidence and its limits.

## Boot and GDAT

An incomplete profile remains at the launcher and does not clear the viewport
or paint generated floor/ceiling art. The boot receipt records source identity,
typed GDAT sections, title/menu, HUD, dungeon, and no-fallback readiness.

The GDAT pipeline exposes `dtPalIRGB`, `dtPalette16`, `dt07` interface/title
records, Rect14 placement, map GRAPHICSSET values, and map-chip assets. The
viewport resolves raw GDAT addresses before decode. Missing mandatory material
is a no-draw receipt, never a substitute. Wall cache keys include graphics
style to prevent material leakage between maps.

## Runtime

M10 owns party, door, item, creature, projectile, actuator, and dungeon state.
The viewport consumes typed plans. Outdoor routes pass the active source sky,
ground, and weather state; indoor routes are clear. Rain, mist, and thunder do
not create procedural pixels without a verified original overlay asset.

## Feature queue status (Q-DM2-01 through Q-DM2-10)

The ten DM2 feature lanes have focused coverage:

| Lane | Scope | Evidence |
|------|-------|----------|
| Q-DM2-01 | GDAT core renderer | 58 GDAT source files, 76 tests (69 wired), 100% pass |
| Q-DM2-02 | GDAT material families | Wall tiles, door variants, stairs, pit, palette, material pairs tested |
| Q-DM2-03 | Creature renderer | 22 creature tests pass |
| Q-DM2-04 | G1 map and c_record runtime | 19 G1 tests pass |
| Q-DM2-05 | SKSAVE interop | 4 save tests pass |
| Q-DM2-06 | Menu, title and audio | 5 startup + 2 menu + 4 sound tests |
| Q-DM2-07 | Party, inventory and spells | 5 spell + 1 champion tests |
| Q-DM2-08 | Creature AI and combat | 3 combat + 7 projectile tests |
| Q-DM2-09 | CCM and world scripts | 5 CCM tests pass |
| Q-DM2-10 | Outdoor scenes and end-to-end play | 3 outdoor + 13 weather tests pass |

The work continues as continuous lanes
(Lane A through E) rather than discrete queue items. Active lanes:

- **Lane A — SkWinCore symbol audit.** Closes `MISSING` rows in
  `docs/reference/audits/SKPROJECT_DM2_NAMED_SYMBOL_AUDIT.tsv` in small
  batches, source-locking each `skproject`/`SKULLWIN` symbol against
  `SKWIN/SkWinCore.cpp` and `SkGlobal.cpp` before implementation. The current
  disposition is the TSV itself; do not copy a historical row count into a
  status summary.
- **Lane B — Audible playback backend.** `dm2_v1_sound_bind_gdat_loader()`
  wires `DM2_PLAY_MUSIC`, `DM2_PLAY_SOUND`, and `DM2_QUERY_SND_ENTRY_INDEX`
  to real GDAT sound data with an SDL3 mixing backend (6000 Hz U8 mono
  stream, additive `sdlAudMix`-shaped mixing). See [DM2 GDAT
  Internals](DM2-GDAT-Internals) for the PCM decode and voice-allocation
  detail. Remaining: wire the backend binding into the live M11 DM2 runtime
  path (app-side integration, not source-lock work).
- **Lane C — Real-data startup/dungeon gate repair.** Fixed the DM2
  real-data gate tests (2 failing as of v3.0.181, was 13) by refining the
  `g1_w0_chains_disabled` flag: real PC G1 `DUNGEON.DAT` disables `w0` chain
  traversal (proven game data, not next-links), while synthetic skproject
  test fixtures preserve `w0` chain traversal so existing fixture-based
  tests keep working. `record_graph_complete` is now enabled for
  skproject-loaded fixtures, unblocking wall-gfx/actuator/3D93B chain
  traversal in those fixtures.
- **Lane D — Creature/cloud real-data passes.** Source-locked the
  `_4976_5aa4` occupancy grid and `DRAW_FLYING_ITEM` selection rules against
  `SKWIN/SkWinCore.cpp` (`QUERY_CREATURE_5x5_POS`, `DRAW_STATIC_OBJECT`'s
  occupancy walk, `DRAW_FLYING_ITEM`) and `SkGlobal.cpp` tables. See [DM2
  GDAT Internals](DM2-GDAT-Internals) for the creature V5 animation chain
  and occupancy-grid detail.
- **Lane E — Real-data combat and drops mechanics.** New
  `dm2_v1_drops_resolve_gdat_creature_drops()` reads the eleven-entry
  per-creature drop table from real GDAT data and resolves proven drop
  words with RNG-gated selection, matching `SkWinCore.cpp`'s creature-death
  drop route. Covers the defense route and combat-drop resolution path. The
  verified CREATURES→v1d296c AIDefinition defense byte is retained in the
  combat receipt, while the complete champion/hand/CMDSTR/RNG damage owner
  remains fail-closed. Tested by `test_dm2_v1_drops_gdat_real_data`
  (skip-safe against real data).

Source reference for all DM2 work: `skproject`
(`SKULLWIN/SkWinCore.cpp`, `SkGlobal.cpp`, plus the per-file `c_*.cpp`
modules named in the symbol audit).

## Verification

```bash
cmake --build build --target test_dm2_v1_boot_profile_smoke \
  test_dm2_v1_runtime_handoff_smoke test_dm2_v1_weather_no_synthetic_overlay \
  --parallel
./build/test_dm2_v1_boot_profile_smoke
./build/test_dm2_v1_runtime_handoff_smoke
./build/test_dm2_v1_weather_no_synthetic_overlay
```

Real-data gate and lane-specific tests:

```bash
cmake --build build --target test_dm2_v1_drops_gdat_real_data \
  test_dm2_v1_creature_occupancy_flying_item test_dm2_v1_sound_gdat_real_data \
  --parallel
./build/test_dm2_v1_drops_gdat_real_data
./build/test_dm2_v1_creature_occupancy_flying_item
./build/test_dm2_v1_sound_gdat_real_data
```

For the GDAT record chain, interface tables, and host receipts, see [DM2 GDAT
Internals](DM2-GDAT-Internals).
