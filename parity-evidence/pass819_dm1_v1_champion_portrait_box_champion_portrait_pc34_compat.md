# pass819 DM1 V1 Champion-Portrait-Box-Champion-Portrait

- Status: PASS819_DM1_V1_BOX_CHAMPION_PORTRAIT_LOCKED
- Gate: Graphics.dat item 562 init var G0047_auc_Graphic562_Box_ChampionPortrait[4] = {0, 31, 0, 28}. The {X, Y, W, H} byte-coordinate sub-rectangle used by REVIVE.C F0132_VIDEO_Blit to extract a single champion portrait (31 bytes wide x 28 bytes tall) from the C026_GRAPHIC_CHAMPION_PORTRAITS source bitmap.
- Runtime assertion floor: 59 assertions in `tests/test_dm1_v1_champion_portrait_box_champion_portrait_pc34_compat.c`.
- Expected test output: `59/59 assertions passed`.

## ReDMCSB Anchors

- DATA.C:85
- DATA.C:424
- DATA.C:1098
- REVIVE.C:142
- REVIVE.C:146

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-818 (Graphics.dat init-table gates batches 1+2+3+4+5+6).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_portrait_box_champion_portrait_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass819_dm1_v1_champion_portrait_box_champion_portrait_pc34_compat/manifest.json`
