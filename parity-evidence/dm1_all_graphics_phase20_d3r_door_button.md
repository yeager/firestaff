# DM1 all-graphics phase 20 — D3R side-door button

Date: 2026-04-25 12:36 Europe/Stockholm
Scope: Firestaff V1 / DM1 D3R side-door button.

## Change

Completed the door-button coverage that ReDMCSB calls directly in `DUNVIEW.C` by adding the D3R side-door button case:

- source graphic: `M634_GRAPHIC_FIRST_DOOR_BUTTON = 453`
- view button index: `C0_VIEW_DOOR_BUTTON_D3R`
- zone: `C1950_ZONE_DOOR_BUTTON + 0`
- resolved/scaled placement: `x=197 y=39 w=4 h=4`

The path only draws when the sampled `D3R` square (`relForward=3`, `relSide=1`) is a non-open door with a door thing whose `button` bit is set.

## Source anchors

- `DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton`
- `DUNVIEW.C F0120_DUNGEONVIEW_DrawSquareD3R`
- `C0_VIEW_DOOR_BUTTON_D3R`
- `C1950_ZONE_DOOR_BUTTON`
- `M634_GRAPHIC_FIRST_DOOR_BUTTON = 453`

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

- `verification-m11/dm1-all-graphics/phase20-d3r-door-button-20260425-1236/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase20-d3r-door-button-20260425-1236/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase20-d3r-door-button-20260425-1236/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no obvious regression in corrected VGA screenshot.

## Remaining work

- Apply D2/D3 button palette-change tables (`G0198/G0199`) instead of simple nearest-neighbor scaling.
- Add door ornaments (`F0109_DUNGEONVIEW_DrawDoorOrnament`).
- Add destroyed-door mask (`M649_GRAPHIC_DOOR_MASK_DESTROYED`).
