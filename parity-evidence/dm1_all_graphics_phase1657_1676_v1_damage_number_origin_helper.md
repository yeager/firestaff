# DM1 all-graphics parity — phase 1657–1676: V1 damage-number origin helper

## Scope

Make the champion damage-number text origin probe-visible and tied to the same source-backed status-box/damage-banner geometry used by the C015 damage overlay.

## Source anchors

- GRAPHICS.DAT `C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL`: `45×7`.
- GRAPHICS.DAT `C007/C008` champion status-box footprint: `67×29`.
- ReDMCSB `CHAMPION.C F0291` draws the damage banner centered over the champion status box, with the numeric damage text over that banner.
- Existing helper `M11_GameView_GetV1DamageIndicatorZone(...)` resolves slot 0 as `(23,171,45,7)` and slot 3 as `(230,171,45,7)`.

## Implemented

- Added `M11_GameView_GetV1DamageNumberOrigin(...)`.
- Routed V1 damage-number drawing through the helper.
- Added invariant coverage for slot 0 and slot 3 damage-number origins.
- V2/prebaked HUD positioning remains unchanged.

## New invariant

- `INV_GV_15U`: V1 champion damage number origin is centered over the C015 damage banner.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_15U V1 champion damage number origin is centered over the C015 damage banner
```
