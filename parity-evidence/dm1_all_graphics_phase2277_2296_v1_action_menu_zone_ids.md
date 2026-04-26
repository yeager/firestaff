# DM1 all-graphics parity — phase 2277–2296: V1 action menu zone ids

## Scope

Expose the source zone ids used by the V1 action menu header and action-name rows, then route row geometry through those ids.

## Source anchors

`ACTIDRAW.C:F0387_MENUS_DrawActionArea` menu branch prints:

- acting champion name through source zone `80`.
- action row names through source zones `85`, `86`, and `87`.

The current reconstructed geometry remains:

- header: `(224,47,87,9)`
- row 0: `(224,58,87,9)`
- row 1: `(224,69,87,9)`
- row 2: `(224,80,87,9)`

## Implemented

- Added `M11_GameView_GetV1ActionMenuHeaderZoneId()`.
- Added `M11_GameView_GetV1ActionMenuRowZoneId()`.
- Routed row-zone geometry through the exposed source zone id mapping.

## Updated invariants

- `INV_GV_300G`: action menu header zone exposes F0387 source zone 80 geometry.
- `INV_GV_300F`: action menu row zones expose F0387 source zones 85-87 geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_300G action menu header zone exposes F0387 source zone 80 geometry
PASS INV_GV_300F action menu row zones expose F0387 source zones 85-87 geometry
```
