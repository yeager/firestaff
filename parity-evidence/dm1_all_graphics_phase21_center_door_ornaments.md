# DM1 all-graphics phase 21 — center door ornaments

Date: 2026-04-25 12:47 Europe/Stockholm
Scope: Firestaff V1 / DM1 center door ornaments.

## Change

Added first source-bound center-door ornament rendering and fixed the door ornament graphic base.

### Fixed base index

Old Firestaff constant was wrong:

- `M11_GFX_DOOR_ORNAMENT_BASE = 165`

Source-backed value:

- `M617_GRAPHIC_FIRST_DOOR_ORNAMENT = 441`

The per-map door ornament table (`doorOrnamentIndices`) now resolves against base 441.

### Center-door ornament path

For visible center doors (`D1C`, `D2C`, `D3C`) with `doorOrnamentOrdinal > 0`, the renderer now:

1. Resolves per-map ornament global index via `m11_ensure_ornament_cache(...)`.
2. Maps global index to original graphic `441 + globalIndex`.
3. Uses ReDMCSB coordinate-set mapping:
   - `G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices = {0,1,1,1,0,1,2,1,1,1,1,1}`
4. Scales like `F0109_DUNGEONVIEW_DrawDoorOrnament`:
   - D1: native (`32/32`)
   - D2: `C21_SCALE_`
   - D3: `C14_SCALE_`
5. Places the ornament relative to the door panel, matching the original behavior where ornaments are drawn into the temporary door bitmap before the door panel is blitted to the viewport.

This intentionally covers center doors first. Side-door ornaments still need explicit handling.

## Source anchors

- `DUNVIEW.C F0109_DUNGEONVIEW_DrawDoorOrnament`
- `DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor`
- `M617_GRAPHIC_FIRST_DOOR_ORNAMENT = 441`
- `G0196_auc_Graphic558_DoorOrnamentCoordinateSetIndices`
- `C2000_ZONE_DOOR_ORNAMENT`
- `C0/C1/C2_VIEW_DOOR_ORNAMENT_D3/D2/D1LCR`
- `C14_SCALE_`, `C21_SCALE_`, `M078_SCALED_DIMENSION`

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

- `verification-m11/dm1-all-graphics/phase21-center-door-ornaments-20260425-1247/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase21-center-door-ornaments-20260425-1247/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase21-center-door-ornaments-20260425-1247/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression or ornament misplacement in the sampled corrected-VGA frame. Ornament visibility in this scene is subtle, so deterministic ornament-specific captures remain needed.

## Remaining work

- Side-door ornaments.
- Destroyed-door mask (`C15_DOOR_ORNAMENT_DESTROYED_MASK` / graphic 439 path).
- D2/D3 palette-change tables for ornaments (`G0200/G0201`).
- Deterministic capture focused on a door with a known ornament ordinal.
