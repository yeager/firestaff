# pass812 DM1 V1 Champion-Panel-Box-Water

- Status: PASS812_DM1_V1_CHAMPION_PANEL_BOX_WATER_LOCKED
- Gate: Graphics.dat item 562 init var G0036_ai_Graphic562_Box_Water[4] = {112, 159, 83, 91}. The {X, Y, W, H} pixel-coordinate rectangle for the water status label blit (paired with food label G0035, same X). PANEL.C F0344_INVENTORY_DrawPanel calls M519_F0020_MAIN_BlitToViewport(C031_GRAPHIC_WATER_LABEL, G0036, byteWidth, C12_COLOR_DARKEST_GRAY, 9) with byte widths C024/C032/C024 for English/German/French localizations.
- Runtime assertion floor: 48 assertions in `tests/test_dm1_v1_champion_panel_box_water_pc34_compat.c`.
- Expected test output: `48/48 assertions passed`.

## ReDMCSB Anchors

- DATA.C:42
- DATA.C:318
- DATA.C:1036
- PANEL.C:1586/1590/1594

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-811 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_water_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass812_dm1_v1_champion_panel_box_water_pc34_compat/manifest.json`
