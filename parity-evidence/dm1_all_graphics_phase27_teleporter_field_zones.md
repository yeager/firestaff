# DM1 all-graphics phase 27 — source-bound teleporter field zones

Date: 2026-04-25 14:08 Europe/Stockholm
Scope: Firestaff V1 / DM1 visible teleporter field rendering.

## Change

Added a source-bound first pass for visible teleporter fields.

The renderer now checks sampled viewport cells for `DUNGEON_ELEMENT_TELEPORTER` with both visibility/open bits set (`MASK0x0004_TELEPORTER_VISIBLE`, `MASK0x0008_TELEPORTER_OPEN`) and draws the original DM1 teleporter field graphic (`C076_GRAPHIC_FIELD_TELEPORTER`) into the ReDMCSB field zones.

Implementation details:

- Adds `m11_draw_dm1_field_zone(...)`:
  - tiles the 32x32 teleporter field graphic across the resolved destination zone
  - applies field transparent color `0x0a` where relevant
  - applies the field mask bitmap family `M652_GRAPHIC_FIRST_FIELD_MASK` (`70..75`) when `G0188` specifies one
  - supports horizontal mask flip via bit `0x80`
- Adds `m11_draw_dm1_teleporter_fields(...)` over all visible D0..D3 teleporter field positions.

This is intentionally deterministic: the original `F0133_VIDEO_BlitBoxFilledWithMaskedBitmap` adds small random phase offsets every frame. For parity-progress screenshots and probes, Firestaff currently uses a stable base offset from `G0188` instead of introducing nondeterministic shimmer.

## Source anchors

- `DUNVIEW.C F0113_DUNGEONVIEW_DrawField`
- `G0188_aauc_Graphic558_FieldAspects`
- `G2035_ac_ViewSquareIndexToFieldAspectIndex`
- `C076_GRAPHIC_FIELD_TELEPORTER`
- `M652_GRAPHIC_FIRST_FIELD_MASK`
- wall-zone family `C702..C717`
- teleport square bits:
  - `MASK0x0004_TELEPORTER_VISIBLE`
  - `MASK0x0008_TELEPORTER_OPEN`

## Covered positions

- D3L2 / D3R2
- D3L / D3C / D3R
- D2L2 / D2R2
- D2L / D2C / D2R
- D1L / D1C / D1R
- D0L / D0C / D0R

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

- `verification-m11/dm1-all-graphics/phase27-teleporter-field-zones-20260425-1408/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase27-teleporter-field-zones-20260425-1408/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase27-teleporter-field-zones-20260425-1408/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression, palette corruption, or field bleed into UI in the sampled corrected-VGA frame.

## Remaining work

- Add a deterministic teleporter-focused capture scene to prove all D0..D3 placements.
- Decide whether to model the original random shimmer phase in runtime while keeping probes deterministic.
- Fluxcage field (`C077_GRAPHIC_FIELD_FLUXCAGE`) remains separate and is not implemented in this pass.
