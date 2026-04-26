# DM1 all-graphics parity — phase 1697–1716: V1 action-icon palette helper

## Scope

Make the DM1 action-hand object-icon palette rewrite explicit and probe-visible.

## Source anchors

- ACTIDRAW.C applies `G0498` palette changes for action-area object icons.
- The action-hand cells recolor object-icon color `12` (dark gray) to cyan for the right-side action area.
- Inventory/status-hand object icons are not the action area and must not use this palette rewrite.
- Palette indices are stored in low nybbles, so both `12` and e.g. `28` (`0x1c`) resolve to the same color-12 remap case.

## Implemented

- Added `M11_GameView_MapV1ActionIconPaletteColor(...)`.
- Routed `m11_draw_dm_object_icon_index(...)` through the helper instead of doing the remap inline.
- Added invariant coverage for action-enabled remap, non-action preservation, high-nybble color-12 remap, and unrelated-color preservation.
- Drawing output remains unchanged.

## New invariant

- `INV_GV_300L`: V1 action object icon palette remaps color-12 nybbles to cyan only in action cells.

## Verification

- `cmake --build build --target firestaff_m11_game_view_probe firestaff -- -j2`
- `FIRESTAFF_DATA="$HOME/.firestaff/data" ./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"`
- `ctest --test-dir build --output-on-failure`

## Probe excerpt

```text
PASS INV_GV_300L V1 action object icon palette remaps color-12 nybbles to cyan only in action cells
```
