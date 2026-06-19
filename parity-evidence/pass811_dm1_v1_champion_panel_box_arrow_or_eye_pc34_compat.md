# pass811 DM1 V1 Champion-Panel-Box-Arrow-Or-Eye

- Status: PASS811_DM1_V1_CHAMPION_PANEL_BOX_ARROW_OR_EYE_LOCKED
- Gate: Graphics.dat item 562 init var G0033_ai_Graphic562_Box_ArrowOrEye[4] = {83, 98, 57, 65}. The {X, Y, W, H} pixel-coordinate rectangle for the chest-content arrow/eye indicator blit. PANEL.C F0344_INVENTORY_DrawPanel calls M519_F0020_MAIN_BlitToViewport(C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT or C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION, G0033, C008_BYTE_WIDTH, C08_COLOR_RED, 9) — the arrow when not pressing eye, the eye when pressing eye.
- Runtime assertion floor: 51 assertions in `tests/test_dm1_v1_champion_panel_box_arrow_or_eye_pc34_compat.c`.
- Expected test output: `51/51 assertions passed`.

## ReDMCSB Anchors

- DATA.C:39
- DATA.C:315
- DATA.C:1032
- PANEL.C:511

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-810 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_arrow_or_eye_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass811_dm1_v1_champion_panel_box_arrow_or_eye_pc34_compat/manifest.json`
