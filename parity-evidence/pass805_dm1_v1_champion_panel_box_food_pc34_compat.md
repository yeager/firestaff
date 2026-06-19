# pass805 DM1 V1 Champion-Panel-Box-Food

- Status: PASS805_DM1_V1_CHAMPION_PANEL_BOX_FOOD_LOCKED
- Gate: Graphics.dat item 562 init var G0035_ai_Graphic562_Box_Food[4] = {112, 159, 60, 68}. The {X, Y, W, H} pixel-coordinate rectangle for the food status label blit onto the champion panel. PANEL.C F0344_INVENTORY_DrawPanel calls M519_F0020_MAIN_BlitToViewport(C030_GRAPHIC_FOOD_LABEL, G0035, byteWidth, ...) with byte widths C024/C032/C048 for the three localizations.
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_champion_panel_box_food_pc34_compat.c`.
- Expected test output: `52/52 assertions passed`.

## ReDMCSB Anchors

- DATA.C:41
- DATA.C:317
- DATA.C:1035
- PANEL.C:1585/1589/1593

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + auto-chest + chest-open-stack-split).
- Not pass798 (icon-graphic-first-icon-index).
- Not pass800 (slot-boxes).
- Not pass801 (light-power-to-light-amount).
- Not pass802 (palette-index-to-light-amount).
- Not pass803 (ordered-cells-to-attack).
- Not pass804 (charge-count-to-torch-type).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_food_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass805_dm1_v1_champion_panel_box_food_pc34_compat/manifest.json`
