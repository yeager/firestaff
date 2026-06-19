# pass817 DM1 V1 Champion-Portrait-Box-Mouth

- Status: PASS817_DM1_V1_CHAMPION_PORTRAIT_BOX_MOUTH_LOCKED
- Gate: Graphics.dat item 562 init var G0048_ai_Graphic562_Box_Mouth[4] = {55, 72, 12, 29}. The {X, Y, W, H} pixel-coordinate rectangle for the champion-portrait mouth-overlay blit. CHAMDRAW.C F0294_CHAMPION_DrawChampionPortraitMouth calls M519_F0020_MAIN_BlitToViewport(nativeBitmap, G0048, C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY, 18) to blit the mouth graphic on top of the portrait when the champion is mid-action.
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_champion_portrait_box_mouth_pc34_compat.c`.
- Expected test output: `52/52 assertions passed`.

## ReDMCSB Anchors

- DATA.C:79
- DATA.C:423
- DATA.C:1095
- CHAMDRAW.C:914

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-816 (Graphics.dat init-table gates batches 1+2+3+4+5).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_portrait_box_mouth_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:423/1095, CHAMDRAW.C:914 in next source refresh.

Manifest: `parity-evidence/verification/pass817_dm1_v1_champion_portrait_box_mouth_pc34_compat/manifest.json`
