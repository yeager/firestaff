# DM1 all-graphics parity — phase 45: object G2030 scale table pinned

Date: 2026-04-25 18:25 Europe/Stockholm

## Goal

Pin DM1's source object scale table so the upcoming exact C2500 object-zone pass has a verified source-backed helper instead of reintroducing invented distance percentages.

This pass intentionally does not yet change visible object placement/scaling; exact application still needs the source view-cell/zone path.

## Source anchors

- `../redmcsb-output/I34E_I34M/DUNVIEW.C`
  - `G2030_auc_ObjectScales[5] = { 27, 21, 18, 14, 12 }`
  - `F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF`
  - object scale index path around `AL0150_ui_ShiftSetIndex`

## Implemented

- `m11_game_view.c`
  - Added `m11_object_source_scale_units(scaleIndex)` backed by source `G2030_auc_ObjectScales`.
  - Clamps out-of-range scale bucket inputs to the valid 0..4 table range.

- `m11_game_view.h`
  - Exposed `M11_GameView_GetObjectSourceScaleUnits(...)` for probes.

- `probes/m11/firestaff_m11_game_view_probe.c`
  - Added `INV_GV_114C` asserting the exact source table values: `27, 21, 18, 14, 12`.

## Verification

Commands run:

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- `PASS INV_GV_114C object source scale units match G2030 table`
- Probe summary: `380/380 invariants passed`
- CTest: `4/4 PASS`

Warnings observed and non-blocking:

- `m11_game_view.c:728:23: warning: unused function 'm11_get_square_ptr'`
- probe warnings for unused/set-but-unused local debug variables around focused captures and projectile setup.

## Remaining work

- Apply `G2030` through the exact source scale-index computation once object drawing is moved to the real `C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES` path.
- Bind object pile shifts through `G0217` + `G0223`.
- Add side-lane and multi-object focused visual gates.
