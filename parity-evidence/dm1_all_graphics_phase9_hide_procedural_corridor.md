# DM1 all-graphics phase 9 — hide procedural corridor in normal V1

Date: 2026-04-25 11:12 Europe/Stockholm
Scope: Firestaff V1 viewport parity cleanup.

## Change

Normal V1 no longer draws Firestaff's procedural corridor/trapezoid renderer over the DM1 floor/ceiling base.

The following invented renderer pieces are now debug-HUD only:

- `m11_draw_corridor_frame`
- `m11_draw_side_feature`
- `m11_draw_wall_face`
- `m11_draw_wall_contents`

Reason: original DM1 does not draw the viewport by procedural trapezoids. ReDMCSB builds `G0296_puc_Bitmap_Viewport` by drawing source graphics into layout zones from `GRAPHICS.DAT` layout 696, especially the wall zones around `C702..C717`, then blits the whole viewport to screen.

Keeping the fake corridor in normal V1 overdraws the now-correct floor/ceiling base and makes the screenshot look more confidently wrong.

## Source anchors for the replacement path

- `DUNVIEW.C F0097_DUNGEONVIEW_DrawViewport`
- `DUNVIEW.C F0792_DUNGEONVIEW_DrawBitmapYYY`
- `DUNVIEW.C F0635_` zone placement path via `COORD.C`
- `C702_ZONE_WALL_D3L2` .. `C717_ZONE_WALL_D0R`
- wall-set graphics:
  - `M661_GRAPHIC_WALLSET_0_D0R = 93`
  - `M662_GRAPHIC_WALLSET_0_D0L = 94`
  - `C095_GRAPHIC_WALLSET_0_D1R = 95`
  - `C096_GRAPHIC_WALLSET_0_D1L = 96`
  - `C097_GRAPHIC_WALLSET_0_D1C = 97`
  - `C098_GRAPHIC_WALLSET_0_D2R2 = 98`
  - `C099_GRAPHIC_WALLSET_0_D2L2 = 99`
  - `C100_GRAPHIC_WALLSET_0_D2R = 100`
  - `C101_GRAPHIC_WALLSET_0_D2L = 101`
  - `C102_GRAPHIC_WALLSET_0_D2C = 102`
  - `C103_GRAPHIC_WALLSET_0_D3R2 = 103`
  - `M663_GRAPHIC_WALLSET_0_D3L2 = 104`
  - `C105_GRAPHIC_WALLSET_0_D3R = 105`
  - `C106_GRAPHIC_WALLSET_0_D3L = 106`
  - `C107_GRAPHIC_WALLSET_0_D3C = 107`

## Artifacts

Generated screenshot set:

- `verification-m11/dm1-all-graphics/phase9-hide-procedural-corridor-20260425-1112/normal/*.pgm`
- `verification-m11/dm1-all-graphics/phase9-hide-procedural-corridor-20260425-1112/normal/*.png`

Quick crop:

- `verification-m11/dm1-all-graphics/phase9-hide-procedural-corridor-20260425-1112/normal/party_hud_top_190_crop.png`

Visual inspection: less fake geometry, but still not 1:1. This intentionally leaves the viewport sparse/unfinished until the original wall-zone blits are ported.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Next step

Implement a small DM1 zone resolver equivalent to `F0635_` for layout 696, then blit wall-set graphics 93..107 into zones `C702..C717` in the same order as `DUNVIEW.C` before re-enabling source-bound normal V1 wall drawing.
