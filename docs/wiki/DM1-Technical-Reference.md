# Dungeon Master (DM1) Technical Reference

## Scope

DM1 V1 targets the PC 3.4 data model. Behavior is source-locked to ReDMCSB:
`DUNGEON.C`, `GROUP.C`, `COMMAND.C`, `TIMELINE.C`, `DUNVIEW.C`, `DRAWVIEW.C`,
`ENTRANCE.C`, and `LOADSAVE.C`. Generated art is never used when a verified
original asset is available.

## Data and Layers

The recursive scanner hashes `GRAPHICS.DAT` and `DUNGEON.DAT`; filenames and
folder layout are not trusted. Archive-backed files are materialized before the
runtime opens them. M10 owns dungeon state, Thing chains, sensors, timeline,
and saves. M11 owns SDL input and presentation. M12 owns launch selection.

The V1 surface is 320x200 indexed. V2 presentation modes remain display
pipelines and must not change original palette indices, timers, dungeon state,
or save bytes.

## Dungeon, AI, and Rendering

Live chain mutation uses the source-shaped F0267 unlink/append path. Cross-map
moves repair both Thing chains and restore caller map context. Timeline handlers
retain source ordering; unsupported semantics are blocked rather than guessed.

Group sight follows ReDMCSB F0200/F0197/F0199/F0227: cone direction,
fixed-point diagonal traversal, two-corner blocking, wall/fakewall/door rules,
Portcullis/Ra exceptions, palette darkness, invisibility, and SEE_INVISIBLE.

M10 supplies the bounded static Thing chain and typed live projectile/explosion
lists. DM1 builds the F0115 summary and layer plan; M11 only consumes the
receipt. HoC mirror candidates are not ordinary dungeon Things and are excluded
from item rendering.

## Saves

PC34 imports stage header, party, active groups, event queue, timeline, and
optional dungeon tail in a candidate world. Publication occurs only after all
ranges, checksums, and queue indexes validate. A bad EVENT/TIMELINE import
leaves the active queue untouched.

## Q-DM1 Queue Status (Q-DM1-01 through Q-DM1-10)

All ten items in the DM1 priority implementation queue are complete:

- **Q-DM1-01** PC34 save corpus and round trip
- **Q-DM1-02** HoC presented-frame consumer (6 material lanes: mirror,
  inscription, object, action/spell, palette, viewport coverage)
- **Q-DM1-03** Dungeon viewport material matrix — all 15 F0107 wall
  ornament positions wired (D3C/D3L/D3R/D3L2/D3R2, D2C/D2L/D2R/D2L2/D2R2,
  D1C/D1L/D1R), plus door, mirror, item, creature, projectile, and
  explosion routing
- **Q-DM1-04** Door, sensor and topology runtime (doors, fakewalls, pits,
  teleporters, F0241/F0712 door animation timers)
- **Q-DM1-05** Group/combat timeline (F0190/F0207/F0209 AI, LoS, melee,
  ranged, projectile impacts, wound/poison, death drops via F0190
  possession lists)
- **Q-DM1-06** Inventory interaction matrix (106 inventory modules
  covering chest open/close, pickup/drop, stack split/merge, hand swap,
  cross-champion transfers, encumbrance, capacity limits)
- **Q-DM1-07** Action and spell HUD (C010/C011 typography, cursor icon
  swap, hit routing, cooldown mirror, spell symbol feedback)
- **Q-DM1-08** Startup audio and cadence (SWSH, title, Entrance, music)
- **Q-DM1-09** Input and controller coverage (keyboard, mouse, touch,
  gamepad, fullscreen scaling)
- **Q-DM1-10** New-game and release evidence (F0803/F0433 ownership,
  save round-trip proofs)

Combat, inventory, and door/sensor topology — originally deferred behind
startup/HUD/viewport per the project priority order — are now implemented
alongside them.

### F0115 Object Selector and G0209 Tables

The F0115 first-object native graphic mapper resolves PC34 object material
through ReDMCSB's `DUNVIEW.C G0209` object-aspect tables rather than any
synthetic asset. Weapon rows resolve through `F0141 -> G0237 -> G0209` to
native `GRAPHICS.DAT` entries; the thrown-object C2900 lane, chest alcove
placement (`G2029`/`C2548`), and creature/projectile blits (`C584+`,
`G0221`/`G0222`, `M613`, `F0142`/`M612`) all route through the same
G0209-anchored selector. The route fails closed when a real PC34 blit
cannot be produced — no fallback drawing is substituted.

### Music State Machine (F0740-F0743)

`dm1_v1_f0740_f0743_music_source_pc34_compat` models DM1's music state
machine and is wired into M11 via `dm1MusicSource`/`dm1MusicState`/
`dm1MusicDriver`. SONG.DAT is bound at init; F0742 selects the map track
on stairs/teleporter transitions; F0743 runs the per-tick update; F0740
pauses playback when music is disabled. All 14 original map tracks now
play (the earlier F0741 fail-closed guard limiting playback to the
game-won track C2 has been removed).

### Wall Ornament Ordinals

Random wall/floor ornaments are computed by F0169/F0170/F0171
(`dm1_v1_random_ornament_pc34_compat.h`, also
`dm1_v1_random_ornament_f0169_f0170_f0171_pc34_compat.h`) using
`ornamentRandomSeed` and per-map `randomWall`/`FloorOrnamentCount` from
the dungeon header. Sensor-driven ornaments are resolved by walking the
thing list and reading `sensor.ornamentOrdinal`. The provider module
`dm1_v1_viewport_wall_ornament_ordinal_provider_pc34_compat.h` bridges
both sources (sensor things first, then random ornaments) into a single
`DM1_ViewportWallOrnamentOrdinalCallback` usable with
`DungeonThings_Compat`. A parallel CSB resolver works directly against
raw `CSB_V1_DungeonData` bytes without the DM1 adapter and is wired into
`fs_game_render_viewport`'s CSB path.

### Cursor Icon Swap

`dm1_v1_cursor_icon_swap_pc34_compat.h` resolves the active cursor
bitmap (arrow / hand / object / champion pointer) for the action and
spell HUD, feeding the action/spell material lane consumed by the HoC
presented-frame consumer.

## Parity Evidence Corpus

`parity-evidence/` holds 1,090+ pass-numbered documents (`pass{NNN}_*.md`),
764 of which are DM1-specific. They cover viewport wall/door/ornament
routing, movement completion (`pass402`, `pass406`, and the viewport
movement completion matrix), inventory slot placement and drag/drop,
combat timelines, champion panel material, and original-transcript
capture gates (row preflight, turn redraw route, live debugger row
gate). Each document anchors its claim to a specific ReDMCSB source
location and a runnable verification path; none substitute a synthetic
capture for original PC34 evidence.

## Save Corpus and New-Game Ownership

The original save corpus runtime gate (see [DM1 PC34
Internals](DM1-PC34-Internals)) independently validates externally
supplied PC34 saves through the F0435 order, exported to transient
memory and reloaded, without ever seeding a start-world dungeon. F0803
(vanilla export, no manifest) and F0433 (Save & Quit / user-save path)
new-game save ownership is proven by round-trip tests in
`test_dm1_v1_original_save_pc34_handoff.c` (byte-identical C3/C4
round-trip), with F0417/F0418 obfuscation and the LSV-02 manifest gate
fully covered. Parity evidence `pass1092` documents the full set of
source anchors for this queue item.

## Verification

```bash
cmake --build build --target firestaff --parallel
./build/test_dm1_v1_group_visible_distance_pc34_compat
./build/test_dm1_v1_projectile_explosion_render_pc34_compat
./build/test_dm1_v1_original_save_pc34_handoff
```

Release visual claims additionally require a real original-data capture through
the packaged application path.

For the detailed PC34 chain, timeline, render-plan, and save invariants, see
[DM1 PC34 Internals](DM1-PC34-Internals).

## ReDMCSB Reference Boundary

ReDMCSB is the primary behavioral reference, but it is a reverse-engineered
source tree rather than FTL's original source. It is especially strong for
control flow, data ownership, and platform-conditioned differences. It is not
by itself proof of a PC 3.4 binary ABI, instruction timing, checksum outcome,
host input ordering, or that a decoded asset is the original asset intended by
a given PC34 route.

Firestaff therefore requires independent PC34 evidence before claiming those
surfaces: hash-identified original executable and data files, provenance
recorded save bytes, decoded asset/palette offsets, and frame/audio/input
captures from the original plus the packaged host application. The open
reference limits are tracked in `TODO.md` as `REDMCSB-DM1-GAP-001` through
`REDMCSB-DM1-GAP-014`. They are audit boundaries, not defects attributed to
ReDMCSB. The added boundaries cover original-bug policy and physical-media
copy protection alongside reconstructed source, compiler, platform, asset,
input, save, and startup evidence.
