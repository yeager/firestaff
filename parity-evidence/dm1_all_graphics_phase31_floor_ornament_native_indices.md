# DM1 all-graphics phase 31 — floor ornament native indices and palette maps

Date: 2026-04-25 14:45 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor ornament bitmap selection.

## Change

Corrected the floor ornament graphic family and native-bitmap increment mapping.

Previous Firestaff code used `247` as the floor ornament base and selected variants with a simple lateral `0..5` mapping. That was wrong for DM1/PC 3.4. In the ReDMCSB source, regular current-map floor ornaments start at `M616_GRAPHIC_FIRST_FLOOR_ORNAMENT = 385`, while footprints are the special pre-base ornament at `379..384`.

Updated:

- `M11_GFX_FLOOR_ORNAMENT_BASE`: `247 -> 385`
- variant selection now follows `G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements`:
  - D3L2/D3R2/D3L/D3R -> increment `0`
  - D3C -> increment `1`
  - D2L/D2R -> increment `2`
  - D2C -> increment `3`
  - D1L/D1R -> increment `4`
  - D1C -> increment `5`
- D3/D2 floor ornaments now use the source palette-change maps:
  - `G0213_auc_Graphic558_PaletteChanges_FloorOrnament_D3`
  - `G0214_auc_Graphic558_PaletteChanges_FloorOrnament_D2`

The placement remains the existing Firestaff rectangle-based floor placement; this pass fixes source bitmap family and variant/palette selection, not final C1500-zone placement.

## Source anchors

- `DEFS.H M616_GRAPHIC_FIRST_FLOOR_ORNAMENT = 385`
- `DEFS.H M639..M753_GRAPHIC_FLOOR_ORNAMENT_15_* = 379..384` for footprints
- `DUNVIEW.C G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements`
- `DUNVIEW.C G0213/G0214 floor ornament palette-change tables`
- `DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament`

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `370/370 invariants passed`
- CTest: `4/4 PASS`

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase31-floor-ornament-native-indices-20260425-1445/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase31-floor-ornament-native-indices-20260425-1445/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase31-floor-ornament-native-indices-20260425-1445/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious palette/UI bleed or floor ornament palette corruption in the sampled corrected-VGA frame.

## Remaining work

- Replace rectangle-based floor ornament placement with source C1500-zone coordinate-set placement.
- Add focused floor-ornament scenes/gates for D1/D2/D3 left/center/right.
- Handle special footprints ornament `15` (`379..384`) explicitly if encountered in map data.
