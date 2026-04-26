# DM1 all-graphics parity — phase 2317–2336: V1 action menu row base zone ids

## Scope

Expose the source parent/base zone ids for the three V1 action-menu rows, alongside the already exposed text/print zones.

## Source anchors

`DEFS.H` names the action row zones before their F0387 print child zones:

- `C082_ZONE_ACTION_AREA_ACTION_0`
- `C083_ZONE_ACTION_AREA_ACTION_1`
- `C084_ZONE_ACTION_AREA_ACTION_2`

The print zones verified in the previous pass are:

- `85`, `86`, `87`

## Implemented

- Added `M11_GameView_GetV1ActionMenuRowBaseZoneId()` returning `82..84`.
- Routed `M11_GameView_GetV1ActionMenuRowZoneId()` validity through the base-zone helper.

## Updated invariant

- `INV_GV_300F`: action menu row zones now verify source base zones `82-84` and print zones `85-87` together.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300F action menu row zones expose F0387 source base zones 82-84 and print zones 85-87 geometry
```
