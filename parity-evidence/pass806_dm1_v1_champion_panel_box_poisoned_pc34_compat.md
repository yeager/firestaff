# pass806 DM1 V1 Champion-Panel-Box-Poisoned

- Status: PASS806_DM1_V1_CHAMPION_PANEL_BOX_POISONED_LOCKED
- Gate: Graphics.dat item 562 init var G0037_ai_Graphic562_Box_Poisoned[4] = {112, 207, 105, 119}. The {X, Y, W, H} pixel-coordinate rectangle for the POISONED status label blit onto the champion panel. PANEL.C F0344_INVENTORY_DrawPanel calls M519_F0020_MAIN_BlitToViewport(C032_GRAPHIC_POISONED_LABEL, G0037, C048_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY, 15) when Champion->PoisonEventCount != 0.
- Runtime assertion floor: 54 assertions in `tests/test_dm1_v1_champion_panel_box_poisoned_pc34_compat.c`.
- Expected test output: `54/54 assertions passed`.

## ReDMCSB Anchors

- DATA.C:43
- DATA.C:319
- DATA.C:1046
- PANEL.C:1603

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
- Not pass805 (champion-panel-box-food).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_poisoned_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass806_dm1_v1_champion_panel_box_poisoned_pc34_compat/manifest.json`
