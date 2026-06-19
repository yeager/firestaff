# pass831 DM1 V1 Box-Action-Area

- Status: PASS831_DM1_V1_BOX_ACTION_AREA_LOCKED
- Gate: Graphics.dat item 562 init var G0001_ai_Graphic562_Box_ActionArea[4] = {224, 319, 77, 121}. The {X, Y, W, H} byte-coordinate sub-rectangle for the action-area background blit on the champion panel. Read sites: ACTIDRAW.C:73/31 + STARTUP2.C:377 (action-area blit + clear + hatch).
- Runtime assertion floor: 54 assertions in `tests/test_dm1_v1_box_action_area_pc34_compat.c`.
- Expected test output: `54/54 assertions passed`.

## ReDMCSB Anchors

- DATA.C:7
- DATA.C:121
- DATA.C:541
- ACTIDRAW.C:73
- ACTIDRAW.C:320
- STARTUP2.C:377

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_action_area_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:121/539, ACTIDRAW.C:73/31, STARTUP2.C:377 in next source refresh.

Manifest: `parity-evidence/verification/pass831_dm1_v1_box_action_area_pc34_compat/manifest.json`
