# DM1 all-graphics phase 36 — source-bound wall ornament zones

Date: 2026-04-25 15:28 Europe/Stockholm
Scope: Firestaff V1 / DM1 wall ornament source-zone rendering.

## Change

Added first source-bound wall ornament zone pass.

Previous phase corrected the wall ornament graphic family (`M615=259`, two native graphics per global wall ornament). This phase wires wall ornaments into DUNVIEW-style wall ornament zones:

- `C1004_ZONE_WALL_ORNAMENT + viewWallIndex`
- coordinate set 0 for the initial DM1 PC table/sample
- transparent color `C10_COLOR_FLESH`
- right-side horizontal flip for source right/left wall views
- native graphic `+1` for applicable front wall views

New renderer:

- `m11_draw_dm1_wall_ornaments(...)`

It samples wall cells, checks `wallOrnamentOrdinal`, resolves the current-map global wall ornament index, then blits the source bitmap into the resolved wall ornament zone.

## Source anchors

- `DEFS.H M615_GRAPHIC_FIRST_WALL_ORNAMENT = 259`
- `DEFS.H C1004_ZONE_WALL_ORNAMENT`
- `DEFS.H C00_VIEW_WALL_D3L2_RIGHT .. M587_VIEW_WALL_D1C_FRONT`
- `DUNVIEW.C F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF`
- `DUNVIEW.C G0194_auc_Graphic558_WallOrnamentCoordinateSetIndices`
- `DUNVIEW.C G0190_auc_Graphic558_WallOrnamentDerivedBitmapIndexIncrement`

## Covered source views

- D3L2 right / D3R2 left
- D3L right / D3R left
- D3L front / D3C front / D3R front
- D2L right / D2R left
- D2L front / D2C front / D2R front
- D1L right / D1R left
- D1C front

## New invariant

Added:

- `INV_GV_38K` — all source-bound wall ornament specs change their wall frames

The focused scene uses sensor-placed wall ornaments and compares each isolated wall+ornament render against the same wall without the ornament.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `373/373 invariants passed`
- CTest: `4/4 PASS`

Relevant output:

```text
PASS INV_GV_38K focused viewport: all source-bound wall ornament specs change their wall frames
```

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase36-wall-ornament-source-zones-20260425-1528/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase36-wall-ornament-source-zones-20260425-1528/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase36-wall-ornament-source-zones-20260425-1528/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious major regression, palette corruption, or UI bleed. Note: the existing bright green vertical marker in the hand/inventory area remains visually suspicious but does not appear related to wall ornament rendering.

## Remaining work

- Model non-zero `G0194` coordinate-set variants if encountered.
- Add palette-change maps for D3/D2 wall ornament derived scaling where appropriate.
- Handle inscription/champion portrait/alcove special behavior from `F0107`.
- Pixel-lock focused wall ornament scenes against original captures.
