# DM1 all-graphics phase 19 — source-bound center door buttons

Date: 2026-04-25 12:25 Europe/Stockholm
Scope: Firestaff V1 / DM1 door button rendering.

## Change

Added a first source-bound path for center/front door buttons.

When a visible center door (`D1C`, `D2C`, `D3C`) has a door thing with `button != 0`, the renderer blits the original door button graphic (`M634_GRAPHIC_FIRST_DOOR_BUTTON = 453`) at the source-resolved door button zone:

- D1C: `C1950_ZONE_DOOR_BUTTON + C3_VIEW_DOOR_BUTTON_D1C`, native 8x9, `x=167 y=43`
- D2C: `C1950_ZONE_DOOR_BUTTON + C2_VIEW_DOOR_BUTTON_D2C`, scaled 5x5, `x=150 y=42`
- D3C: `C1950_ZONE_DOOR_BUTTON + C1_VIEW_DOOR_BUTTON_D3C`, scaled 4x4, `x=137 y=41`

This follows ReDMCSB `F0110_DUNGEONVIEW_DrawDoorButton` at a conservative level. D2/D3 use nearest-neighbor scaled blits for the derived bitmaps; exact palette-change derivation is still TODO.

## Source anchors

- `DUNVIEW.C F0110_DUNGEONVIEW_DrawDoorButton`
- `M634_GRAPHIC_FIRST_DOOR_BUTTON = 453`
- `C1950_ZONE_DOOR_BUTTON`
- `C1_VIEW_DOOR_BUTTON_D3C`
- `C2_VIEW_DOOR_BUTTON_D2C`
- `C3_VIEW_DOOR_BUTTON_D1C`
- `G0197_auc_Graphic558_DoorButtonCoordinateSet[0] = 0`

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

- `verification-m11/dm1-all-graphics/phase19-door-buttons-20260425-1225/normal/party_hud_statusbox_gfx_vga.ppm`
- `verification-m11/dm1-all-graphics/phase19-door-buttons-20260425-1225/normal/party_hud_statusbox_gfx_vga.png`
- `verification-m11/dm1-all-graphics/phase19-door-buttons-20260425-1225/normal/party_hud_top_190_crop_vga.png`

Visual inspection: no new viewport regression visible in the sampled frame. A cyan block in the right-side control panel is visible, but comparison with phase18 shows it was already present before this pass and is not caused by door-button blitting.

## Remaining work

- Add D3R side-door button path (`C0_VIEW_DOOR_BUTTON_D3R`) for the D3R case in `DUNVIEW.C`.
- Apply D2/D3 palette-change tables from `G0198/G0199` instead of only scaling native button pixels.
- Capture a deterministic door-button scene where the button is visible for direct visual proof.
- Continue to door ornaments and destroyed door masks.
