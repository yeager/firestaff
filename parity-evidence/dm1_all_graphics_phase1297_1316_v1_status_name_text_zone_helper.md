# DM1 all-graphics parity — phase 1297–1316: V1 status name text zone helper

## Scope

Harden compact champion status-box name placement by exposing the source layout-696 `C163..C166` text child zones and routing centered name drawing through the shared helper.

## Source anchors

- `zones_h_reconstruction.json` from GRAPHICS.DAT layout `C696`:
  - `C159..C162` are the 42×7 name clear zones at each status-box origin.
  - `C163..C166` are the type-18 name text child zones at parent + `(1,0)`, clipped to `42×7`.
- Firestaff's V1 status boxes use screen origin `(12,160)` and source stride `69`, so text zones resolve to slot 0 `(13,160,42,7)` and slot 3 `(220,160,42,7)`.
- Existing centered-name rendering already matches the visual intent; this pass makes the text zone probe-visible and source-backed rather than only local arithmetic.

## Implemented

- Added `M11_GameView_GetV1StatusNameTextZone(...)` for layout-696 `C163..C166` text geometry.
- Routed V1 compact champion-name drawing through that helper.
- Added invariant coverage for first and fourth champion text zones.

## New invariant

- `INV_GV_15E8`: V1 status name text zones match layout-696 `C163..C166` geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`
- Probe result: `477/477 invariants passed`
- CTest result: `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E8 V1 status name text zones match layout-696 C163..C166 geometry
# summary: 477/477 invariants passed
```
