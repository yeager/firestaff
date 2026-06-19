# pass835 DM1 V1 Box-Title-Strikes-Back-Destination

- Status: PASS835_DM1_V1_BOX_TITLE_STRIKES_BACK_DESTINATION_LOCKED
- Gate: Graphics.dat item 562 init var G0003_ai_Graphic562_Box_Title_StrikesBack_Destination[4] = {0, 319, 118, 174}. The {X, Y, W, H} byte-coordinate sub-rectangle for the Strikes Back title-view backdrop blit during the door-opening animation. Read sites: TITLE.C:233/236 F0132_VIDEO_Blit.
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_title_strikes_back_destination_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:9
- DATA.C:125
- DATA.C:545
- TITLE.C:233/236

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-832 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_title_strikes_back_destination_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass835_dm1_v1_box_title_strikes_back_destination_pc34_compat/manifest.json`
