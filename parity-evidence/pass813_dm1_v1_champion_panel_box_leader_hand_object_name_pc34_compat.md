# pass813 DM1 V1 Champion-Panel-Box-Leader-Hand-Object-Name

- Status: PASS813_DM1_V1_CHAMPION_PANEL_BOX_LEADER_HAND_OBJECT_NAME_LOCKED
- Gate: Graphics.dat item 562 init var G0028_ai_Graphic562_Box_LeaderHandObjectName[4] = {233, 319, 33, 38}. The {X, Y, W, H} pixel-coordinate rectangle for the leader-hand object name black-fill backdrop. OBJECT.C:281 M524_FillScreenBox(G0028, C00_COLOR_BLACK) draws the black backdrop before the leader-hand object name text is rendered on top.
- Runtime assertion floor: 50 assertions in `tests/test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat.c`.
- Expected test output: `50/50 assertions passed`.

## ReDMCSB Anchors

- DATA.C:34
- DATA.C:262
- DATA.C:923
- OBJECT.C:281

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-812 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass813_dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat/manifest.json`
