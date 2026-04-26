# DM1 all-graphics parity — phase 2017–2036: V1 champion identity graphics

## Scope

Expose the source graphics used for champion identity rendering and route portrait/icon loads through probe-visible helpers.

## Source anchors

- `C026_GRAPHIC_CHAMPION_PORTRAITS` is graphic 26, the 256×87 portrait strip used for 32×29 champion portraits.
- `C028_GRAPHIC_CHAMPION_ICONS` is graphic 28, the small champion-icon fallback strip.
- Existing asset-size probes already verify graphic 26 as 256×87; this pass makes the code path itself explicit.

## Implemented

- Added `M11_GameView_GetV1ChampionPortraitGraphicId()` returning graphic 26.
- Added `M11_GameView_GetV1ChampionIconGraphicId()` returning graphic 28.
- Routed inventory/status portrait loading and endgame mirror portrait loading through the new portrait helper.
- Routed the inventory/status small-icon fallback through the new icon helper.

## Updated invariant

- `INV_GV_300S`: V1 champion identity graphics use source C026 portraits and C028 icons.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300S V1 champion identity graphics use source C026 portraits and C028 icons
```
