# DM1 all-graphics parity — phase 2337–2356: V1 status hand zone ids

## Scope

Expose the layout-696 source zone ids for the compact champion ready/action hand slots and route hand-slot geometry through those ids.

## Source anchors

Layout-696 / `DEFS.H` defines the champion status-box hand zones as:

- champion 0: `C211` ready, `C212` action
- champion 1: `C213` ready, `C214` action
- champion 2: `C215` ready, `C216` action
- champion 3: `C217` ready, `C218` action

Ready hand zones are status-box-relative `(4,10)` and action hand zones are `(24,10)`, both `16×16`.

## Implemented

- Added `M11_GameView_GetV1StatusHandZoneId()`.
- Routed `M11_GameView_GetV1StatusHandZone()` through the source zone id mapping.

## Updated invariant

- `INV_GV_15E6`: V1 status hand slot zones expose layout-696 `C211-C218` ids and geometry.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2` — OK
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"` — OK, `# summary: 513/513 invariants passed`
- `ctest --test-dir build --output-on-failure` — OK, `100% tests passed, 0 tests failed out of 5`

## Probe excerpt

```text
PASS INV_GV_15E6 V1 status hand slot zones expose layout-696 C211..C218 ids and geometry
```
