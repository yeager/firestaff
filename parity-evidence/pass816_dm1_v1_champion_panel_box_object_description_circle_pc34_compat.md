# pass816 DM1 V1 Champion-Panel-Box-Object-Description-Circle

- Status: PASS816_DM1_V1_CHAMPION_PANEL_BOX_OBJECT_DESCRIPTION_CIRCLE_LOCKED
- Gate: Graphics.dat item 562 init var G0034_ai_Graphic562_Box_ObjectDescriptionCircle[4] = {105, 136, 53, 79}. The {X, Y, W, H} pixel-coordinate rectangle for the object-description circle blit. PANEL.C F0344_INVENTORY_DrawPanel calls M519_F0020_MAIN_BlitToViewport(C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE, G0034, C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY, 9).
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_champion_panel_box_object_description_circle_pc34_compat.c`.
- Expected test output: `52/52 assertions passed`.

## ReDMCSB Anchors

- DATA.C:40
- DATA.C:316
- DATA.C:1033
- PANEL.C:1141

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-815 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_object_description_circle_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass816_dm1_v1_champion_panel_box_object_description_circle_pc34_compat/manifest.json`
