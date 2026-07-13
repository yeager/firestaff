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
`REDMCSB-DM1-GAP-012`. They are audit boundaries, not defects attributed to
ReDMCSB.
