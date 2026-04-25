# DM1 all-graphics phase 22 — side door ornaments

Date: 2026-04-25 12:56 Europe/Stockholm
Scope: Firestaff V1 / DM1 side door ornaments.

## Change

Extended the source-bound door ornament path from center doors to side doors.

Covered side positions:

- `D3L2` / `D3R2`
- `D3L` / `D3R`
- `D2L` / `D2R`
- `D1L` / `D1R`

For each visible non-open side door with `doorOrnamentOrdinal > 0`, the renderer:

1. Resolves the panel placement, including door opening-state clipping.
2. Resolves per-map ornament global index via `doorOrnamentIndices`.
3. Uses source-backed base `M617_GRAPHIC_FIRST_DOOR_ORNAMENT = 441`.
4. Uses the same coordinate-set mapping as center doors (`G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices`).
5. Places the ornament relative to the clipped side door panel, matching the original temp-door-bitmap model.

This still does not apply the D2/D3 palette-change tables (`G0200/G0201`); that remains TODO.

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

- `verification-m11/dm1-all-graphics/phase22-side-door-ornaments-20260425-1256/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase22-side-door-ornaments-20260425-1256/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase22-side-door-ornaments-20260425-1256/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression, ornament bleed, or right-UI contamination in the sampled corrected-VGA frame.

## Remaining work

- Apply D2/D3 palette-change tables for door ornaments (`G0200/G0201`).
- Add destroyed-door mask (`C15_DOOR_ORNAMENT_DESTROYED_MASK` / graphic 439 path).
- Add deterministic ornament-focused capture scene.
