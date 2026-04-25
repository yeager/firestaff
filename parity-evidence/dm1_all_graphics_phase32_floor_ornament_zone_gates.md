# DM1 all-graphics phase 32 — source-bound floor ornament zone gates

Date: 2026-04-25 14:58 Europe/Stockholm
Scope: Firestaff V1 / DM1 floor ornament source-zone rendering and focused gates.

## Change

Added a source-bound floor ornament viewport pass and focused matrix gate.

The prior phase corrected the floor ornament graphic family and native-bitmap increments (`M616=385`, `G0191`). This phase wires those corrected source graphics into DUNVIEW-style floor ornament zones instead of relying only on the old rectangle-based procedural placement.

New renderer:

- `m11_draw_dm1_floor_ornaments(...)`

It samples visible viewport cells, checks `floorOrnamentOrdinal`, resolves the per-map floor ornament index, then draws the native source bitmap into resolved `C1500_ZONE_FLOOR_ORNAMENT + viewFloorIndex` zones using transparent color `C10_COLOR_FLESH`.

## Source anchors

- `DUNVIEW.C F0108_DUNGEONVIEW_DrawFloorOrnament`
- `DUNVIEW.C G0191_auc_Graphic558_FloorOrnamentNativeBitmapIndexIncrements`
- `DEFS.H M616_GRAPHIC_FIRST_FLOOR_ORNAMENT = 385`
- `DEFS.H C1500_ZONE_FLOOR_ORNAMENT`
- `DEFS.H C00_VIEW_FLOOR_D3L2 .. M596_VIEW_FLOOR_D1R`

## Covered source zones

Using coordinate set 0 (`G0195` is all zero for the DM1 PC floor ornament table):

- `C1501` D3R2
- `C1502` D3L
- `C1503` D3C
- `C1504` D3R
- `C1505` D2L
- `C1506` D2C
- `C1507` D2R
- `C1508` D1L
- `C1509` D1C
- `C1510` D1R

`C1500` D3L2 resolves to a one-pixel strip for ornament 0 where this sample can be fully transparent, so the focused visible-change invariant excludes it. The renderer still has the D3L2 spec.

## New invariant

Added:

- `INV_GV_38I` — all visibly drawable floor ornament positions change their corridor frames

The focused scene uses a sensor-placed floor ornament (`ornamentOrdinal=1`) to avoid depending on random ornament hash selection.

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `371/371 invariants passed`
- CTest: `4/4 PASS`

Relevant output:

```text
PASS INV_GV_38I focused viewport: all visibly drawable floor ornament positions change their corridor frames
```

## Artifacts

Corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase32-floor-ornament-zone-gates-20260425-1458/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase32-floor-ornament-zone-gates-20260425-1458/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase32-floor-ornament-zone-gates-20260425-1458/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious visual regression, palette corruption, or UI bleed. The sampled floor ornament appears contained to floor/source-zone geometry.

## Remaining work

- Implement right-side horizontal flip for floor ornaments (`F0108` flips D3R2/D3R/D2R/D1R and sometimes footprints center views).
- Add special footprint ornament `15` handling (`379..384`) if encountered.
- Pixel-lock focused scenes against original captures.
