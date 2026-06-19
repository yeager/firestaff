# pass836 DM1 V1 Box-Title-Strikes-Back-Source

- Status: PASS836_DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_LOCKED
- Gate: Graphics.dat item 562 init var G0004_ai_Graphic562_Box_Title_StrikesBack_Source[4] = {0, 319, 0, 56}. The {X, Y, W, H} byte-coordinate sub-rectangle for the Strikes-Back title screen source blit. Read sites: TITLE.C:134/137/335/338 F0132_VIDEO_Blit.
- Runtime assertion floor: 53 assertions in `tests/test_dm1_v1_box_title_strikes_back_source_pc34_compat.c`.
- Expected test output: `53/53 assertions passed`.

## ReDMCSB Anchors

- DATA.C:10
- DATA.C:127
- DATA.C:547
- TITLE.C:134/137/335/338

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-835 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_box_title_strikes_back_source_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:127/547, TITLE.C:134/137/335/338 in next source refresh.

Manifest: `parity-evidence/verification/pass836_dm1_v1_box_title_strikes_back_source_pc34_compat/manifest.json`
