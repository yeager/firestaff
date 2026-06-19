# pass838 DM1 V1 Box-Entrance-Opening-Door-Left

- Status: PASS838_DM1_V1_BOX_ENTRANCE_OPENING_DOOR_LEFT_LOCKED
- Gate: Graphics.dat item 562 init var G0007_ai_Graphic562_Box_Entrance_OpeningDoorLeft[4] = {0, 100, 0, 160}. The {X, Y, W, H} byte-coordinate sub-rectangle for the entrance door-opening animation (left half). Read sites: ENTRANCE.C:149 (M769_BOX_RIGHT shrink) + ENTRANCE.C:189/191/194 (F0132_VIDEO_Blit).
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_entrance_opening_door_left_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:13
- DATA.C:133
- DATA.C:553
- ENTRANCE.C:149
- ENTRANCE.C:189/191/194

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-837 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14+15).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_entrance_opening_door_left_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:133/553, ENTRANCE.C:149/189/191/194 in next source refresh.

Manifest: `parity-evidence/verification/pass838_dm1_v1_box_entrance_opening_door_left_pc34_compat/manifest.json`
