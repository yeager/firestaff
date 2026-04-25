# DM1 all-graphics phase 18 — side door opening-state clipping

Date: 2026-04-25 12:10 Europe/Stockholm
Scope: Firestaff V1 / DM1 side door opening states.

## Change

Extended source-bound door opening clipping from center doors to side doors.

For side-door panels, states `1..3` now use the `zone + state` clipping pattern resolved from layout 696 / `COORD.C F0635_`, matching the same `F0111_DUNGEONVIEW_DrawDoor` source rule already applied to center doors:

```c
P2084_i_ZoneIndex += P0125_ui_DoorState;
```

Conservative behavior remains:

- state `0`: open enough to cross, no panel
- states `1..3`: partial clipped panel
- states `4/5`: base/closed-style panel until destroyed-mask and ornament paths are ported

## Covered side positions

- `D3L2` / `D3R2`
- `D3L` / `D3R`
- `D2L` / `D2R`
- `D1L` / `D1R`

Opening-state clipping keeps each side panel's original source X clipping where applicable, and adjusts source Y/height to the resolved partial zones.

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

Generated corrected-VGA screenshot set:

- `verification-m11/dm1-all-graphics/phase18-side-door-opening-states-20260425-1210/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase18-side-door-opening-states-20260425-1210/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase18-side-door-opening-states-20260425-1210/normal/party_hud_top_190_crop_vga.png`

Visual inspection using the corrected VGA export: no obvious side-door clipping or palette regression visible in the sampled frame.

## Next step

Door details that remain before calling doors source-bound enough:

1. destroyed door mask (`M649_GRAPHIC_DOOR_MASK_DESTROYED`)
2. door ornaments (`F0109_DUNGEONVIEW_DrawDoorOrnament`)
3. door buttons (`F0110_DUNGEONVIEW_DrawDoorButton`)
4. vertical-door opening shift branch from `F0111_DUNGEONVIEW_DrawDoor`
