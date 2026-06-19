# pass839 DM1 V1 Box-Entrance-Opening-Door-Right

- Status: PASS839_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_RIGHT_LOCKED
- Gate: Graphics.dat item 562 init var G0008_ai_Graphic562_Box_Entrance_OpeningDoorRight[4] = {109, 231, 0, 160}. The {X, Y, W, H} byte-coordinate sub-rectangle for the entrance door-opening animation (right half). Read sites: ENTRANCE.C:150 (M768_BOX_LEFT shrink) + ENTRANCE.C:190/192/195 (F0132_VIDEO_Blit).
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_entrance_opening_door_right_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:14
- DATA.C:135
- DATA.C:555
- ENTRANCE.C:150
- ENTRANCE.C:190/192/195

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-837 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_entrance_opening_door_right_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:143/553, ENTRANCE.C:149/189/191/194 in next source refresh.

Manifest: `parity-evidence/verification/pass839_dm1_v1_box_entrance_opening_door_right_pc34_compat/manifest.json`
