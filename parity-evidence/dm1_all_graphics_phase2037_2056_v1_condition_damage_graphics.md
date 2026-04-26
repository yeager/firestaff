# DM1 all-graphics parity — phase 2037–2056: V1 condition/damage graphics

## Scope

Expose the source graphic ids for V1 HUD condition and damage overlays, and route the draw paths through those helpers.

## Source anchors

- `C032_GRAPHIC_POISONED_LABEL` is graphic 32, the poisoned status label.
- `C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL` is graphic 15, the status-box damage overlay.
- `C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG` is graphic 16, the inventory portrait damage overlay.
- `C014_GRAPHIC_DAMAGE_TO_CREATURE` is graphic 14, the viewport creature-hit overlay.

## Implemented

- Added `M11_GameView_GetV1PoisonLabelGraphicId()`.
- Added `M11_GameView_GetV1ChampionSmallDamageGraphicId()`.
- Added `M11_GameView_GetV1ChampionBigDamageGraphicId()`.
- Added `M11_GameView_GetV1CreatureDamageGraphicId()`.
- Routed poisoned-label, champion-damage, inventory-damage, and creature-hit overlay loads through the helpers.

## Updated invariant

- `INV_GV_300T`: V1 HUD condition/damage graphics use source C032/C015/C016/C014 ids.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300T V1 HUD condition/damage graphics use source C032/C015/C016/C014 ids
```
