# pass815 DM1 V1 Champion-Panel-Box-Panel

- Status: PASS815_DM1_V1_CHAMPION_PANEL_BOX_PANEL_LOCKED
- Gate: Graphics.dat item 562 init var G0032_ai_Graphic562_Box_Panel[4] = {80, 223, 52, 124}. The {X, Y, W, H} pixel-coordinate rectangle for the champion-panel backdrop blit (vertical panel design: H=124 > W=52). Read sites: PANEL.C:967/1140/1582/1600/1611 + CHEST.C + REVIVE.C blits various panel graphics into G0032's box.
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_champion_panel_box_panel_pc34_compat.c`.
- Expected test output: `52/52 assertions passed`.

## ReDMCSB Anchors

- DATA.C:38
- DATA.C:314
- DATA.C:1031
- PANEL.C:967/1140/1582/1600/1611

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-814 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_panel_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:314/1031, PANEL.C:967/1140/1582/1600/1611 in next source refresh.

Manifest: `parity-evidence/verification/pass815_dm1_v1_champion_panel_box_panel_pc34_compat/manifest.json`
