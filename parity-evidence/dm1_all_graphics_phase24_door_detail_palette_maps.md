# DM1 all-graphics phase 24 — door detail palette-change maps

Date: 2026-04-25 13:30 Europe/Stockholm
Scope: Firestaff V1 / DM1 door button + ornament palette changes.

## Change

Added source-backed palette-change mapping for scaled D2/D3 door buttons and ornaments.

Before this pass, D2/D3 door details were scaled but retained native palette indices. ReDMCSB applies per-depth palette-change tables before drawing derived scaled bitmaps, so the details could be geometrically right but color-wrong.

Implemented a local scaled blit helper:

- `m11_blit_scaled_palette_map(...)`

It applies a 16-entry palette index mapping during nearest-neighbor scaled blits.

## Tables applied

### Door buttons / button-like small details

From `DUNVIEW.C`:

- `G0198_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D3`
  - `{0,0,12,3,4,3,0,6,3,9,10,11,0,1,0,2}`
- `G0199_auc_Graphic558_PaletteChanges_DoorButtonAndWallOrnament_D2`
  - `{0,12,1,3,4,3,6,7,5,9,10,11,0,2,14,13}`

Applied to:

- D3C door button
- D3R door button
- D2C door button

D1C remains native/no palette-change, matching `F0110`.

### Door ornaments

From `DUNVIEW.C`:

- `G0200_auc_Graphic558_PaletteChanges_DoorOrnament_D3`
  - `{0,12,1,3,4,3,0,6,3,9,10,11,0,2,0,13}`
- `G0201_auc_Graphic558_PaletteChanges_DoorOrnament_D2`
  - `{0,1,2,3,4,3,6,7,5,9,10,11,12,13,14,15}`

Applied by `m11_draw_dm1_door_ornament_on_panel(...)` for D3 and D2 ornaments. D1 remains native/no palette-change, matching `F0109`.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `362/362 invariants passed`
- CTest: `4/4 PASS`

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase24-door-detail-palette-maps-20260425-1330/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase24-door-detail-palette-maps-20260425-1330/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase24-door-detail-palette-maps-20260425-1330/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression or palette bleed in the corrected-VGA sampled frame.

## Remaining work

Door detail parity still needs deterministic focused scenes for:

- visible D1/D2/D3 buttons
- visible door ornaments by ordinal/coordinate set
- destroyed-door masks

After that, next major viewport domains are pits/stairs/teleporter/floor-object zones.
