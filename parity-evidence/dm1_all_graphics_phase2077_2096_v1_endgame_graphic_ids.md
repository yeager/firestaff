# DM1 all-graphics parity — phase 2077–2096: V1 endgame graphic ids

## Scope

Expose and reuse the source graphic ids used by the V1 endgame/victory overlay.

## Source anchors

- `C006_GRAPHIC_THE_END` is graphic 6, the original "The End" title used by the victory screen.
- The champion mirror frame used by the endgame champion list is graphic 346.

## Implemented

- Added `M11_GameView_GetV1EndgameTheEndGraphicId()` returning graphic 6.
- Added `M11_GameView_GetV1EndgameChampionMirrorGraphicId()` returning graphic 346.
- Routed endgame overlay draw loads through these helpers.
- Routed existing endgame asset probes through the helpers to keep the ids probe-visible.

## Updated invariant

- `INV_GV_300V`: V1 endgame graphics use source C006 The End and C346 champion mirror ids.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300V V1 endgame graphics use source C006 The End and C346 champion mirror ids
```
