# DM1 all-graphics parity — phase 2437–2456: V1 status hand parent zone ids

## Scope

Expose the layout-696 parent zones for each champion's compact ready/action hand area, and keep the child slot-box zones routed through that parent identity.

## Source anchors

Layout-696 / CHAMDRAW.C `F0291_CHAMPION_DrawSlot` status hand zones:

- parent hand containers: `C207..C210`
- ready/action child slot boxes: `C211..C218`

The child zones are `16×16`, with ready/action offsets `+4,+10` and `+24,+10` inside each champion parent.

## Implemented

- Added `M11_GameView_GetV1StatusHandParentZoneId()` for `C207..C210`.
- Routed `M11_GameView_GetV1StatusHandZoneId()` validation through the parent zone-id helper.

## Updated invariants

- `INV_GV_15E6`: V1 status hand slot zones expose layout-696 `C207-C210` parents and `C211-C218` child ids/geometry.

## Verification

Passed in this batch:

```text
cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2
FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
# summary: 513/513 invariants passed
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 5
```
