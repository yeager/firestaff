# pass837 DM1 V1 Box-Title-Presents

- Status: PASS837_DM1_V1_BOX_TITLE_PRESENTS_LOCKED
- Gate: Graphics.dat item 562 init var G0005_ai_Graphic562_Box_Title_Presents[4] = {0, 319, 90, 105}. The {X, Y, W, H} byte-coordinate sub-rectangle for the Title-Presents title screen source blit. Read sites: TITLE.C:126/129/321/324 F0132_VIDEO_Blit.
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_title_presents_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:11
- DATA.C:129
- DATA.C:549
- TITLE.C:126/129/321/324

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-835 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_title_presents_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass837_dm1_v1_box_title_presents_pc34_compat/manifest.json`
