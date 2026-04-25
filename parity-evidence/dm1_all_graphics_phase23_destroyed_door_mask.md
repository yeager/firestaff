# DM1 all-graphics phase 23 — destroyed-door mask

Date: 2026-04-25 13:06 Europe/Stockholm
Scope: Firestaff V1 / DM1 destroyed-door mask.

## Change

Added a first source-bound destroyed-door mask path for center and side doors.

When a visible door cell has `doorState == 5` (`C5_DOOR_STATE_DESTROYED`), the renderer overlays original graphic:

- `M649_GRAPHIC_DOOR_MASK_DESTROYED = 439`

onto the already resolved visible door panel zone. This follows the source behavior in `F0111_DUNGEONVIEW_DrawDoor`, where destroyed doors call:

```c
F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), viewDoorOrnamentIndex)
```

The current implementation uses the original full-size mask graphic and scales it to the resolved panel zone for:

- center doors: `D1C`, `D2C`, `D3C`
- side doors: `D3L2/R2`, `D3L/R`, `D2L/R`, `D1L/R`

The movement/crossing semantics still treat destroyed doors as open enough via `m11_viewport_cell_is_open(...)`; this pass is render-only.

## Source anchors

- `DEFS.H C5_DOOR_STATE_DESTROYED = 5`
- `DEFS.H M649_GRAPHIC_DOOR_MASK_DESTROYED = 439`
- `DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor`
- `DUNVIEW.C F0109_DUNGEONVIEW_DrawDoorOrnament`
- `C15_DOOR_ORNAMENT_DESTROYED_MASK`

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

- `verification-m11/dm1-all-graphics/phase23-destroyed-door-mask-20260425-1306/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase23-destroyed-door-mask-20260425-1306/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase23-destroyed-door-mask-20260425-1306/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression or UI bleed in the corrected-VGA sampled frame.

## Remaining work

- Replace scaled-mask approximation with exact `F0109` coordinate-set placement once destroyed mask coordinate-set is fully represented in the per-map `G0103` analogue.
- Add D2/D3 palette-change tables for door ornaments/buttons.
- Capture deterministic destroyed-door scene for direct proof.
