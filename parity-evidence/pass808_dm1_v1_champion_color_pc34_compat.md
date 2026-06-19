# pass808 DM1 V1 Champion-Color

- Status: PASS808_DM1_V1_CHAMPION_COLOR_LOCKED
- Gate: Graphics.dat item 562 init var G0046_auc_Graphic562_ChampionColor[4] = {7, 11, 8, 14}. Champion 0 (leader) gets color 7 (LIGHT_GRAY), champion 1 = 11 (LIGHT_CYAN), champion 2 = 8 (LIGHT_RED), champion 3 = 14 (LIGHT_YELLOW). Used for champion-icon/portrait fill and champion-name text color.
- Runtime assertion floor: 44 assertions in `tests/test_dm1_v1_champion_color_pc34_compat.c`.
- Expected test output: `44/44 assertions passed`.

## ReDMCSB Anchors

- DATA.C:84
- DATA.C:423
- DATA.C:1095
- CHAMDRAW.C:48/51/60/300/342/1022
- CHAMPION.C:986/1016/1052
- REVIVE.C:868/872/887

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-807 (Graphics.dat init-table gates batches 1+2+3).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_color_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass808_dm1_v1_champion_color_pc34_compat/manifest.json`
