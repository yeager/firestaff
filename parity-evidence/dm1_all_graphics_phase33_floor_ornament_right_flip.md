# DM1 all-graphics phase 33 — floor ornament right-side flip

Date: 2026-04-25 15:05 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor ornament horizontal mirroring.

## Change

Added horizontal flip support for source-zone blits and applied it to right-side floor ornament views.

ReDMCSB `F0108_DUNGEONVIEW_DrawFloorOrnament` flips floor ornaments for right-side views:

- `C01_VIEW_FLOOR_D3R2`
- `M590_VIEW_FLOOR_D3R`
- `M593_VIEW_FLOOR_D2R`
- `M596_VIEW_FLOOR_D1R`

Implemented:

- `m11_draw_dm1_zone_blit_maybe_flip(...)`
- `flipHorizontal` flag in DM1 floor ornament source-zone specs

This keeps the source-zone placement from phase 32 but makes right-side floor ornament orientation source-faithful.

## Source anchors

- `DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament`
- flip condition covering D3R2/D3R/D2R/D1R
- `C1500_ZONE_FLOOR_ORNAMENT + viewFloorIndex`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `371/371 invariants passed`
- CTest: `4/4 PASS`

Focused floor-ornament gate remains passing:

```text
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
```

## Remaining work

- Special footprint ornament `15` center-view flip handling is still separate.
- Pixel-lock focused floor ornament scenes against original captures.
