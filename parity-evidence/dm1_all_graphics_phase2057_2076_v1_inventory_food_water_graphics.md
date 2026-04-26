# DM1 all-graphics parity — phase 2057–2076: V1 inventory food/water graphics

## Scope

Expose the source graphics used by the V1 inventory panel background and food/water status labels, then route those loads through probe-visible helpers.

## Source anchors

- `C020_GRAPHIC_PANEL_EMPTY` is graphic 20, the inventory panel backdrop.
- `C030_GRAPHIC_FOOD_LABEL` is graphic 30, the 34×9 food label.
- `C031_GRAPHIC_WATER_LABEL` is graphic 31, the 46×9 water label.

## Implemented

- Added `M11_GameView_GetV1InventoryPanelGraphicId()` returning graphic 20.
- Added `M11_GameView_GetV1FoodLabelGraphicId()` returning graphic 30.
- Added `M11_GameView_GetV1WaterLabelGraphicId()` returning graphic 31.
- Routed inventory panel, food label, and water label asset loads through the helpers.

## Updated invariant

- `INV_GV_300U`: V1 inventory panel status uses source C020 panel and C030/C031 food-water labels.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300U V1 inventory panel status uses source C020 panel and C030/C031 food-water labels
```
