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

M11 gives DM1 only its static Thing chain. DM1 classifies active projectile and
explosion instances for the F0115 layer plan. HoC mirror candidates are not
ordinary dungeon Things and are excluded from item rendering.

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
