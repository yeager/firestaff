# pass818 DM1 V1 Champion-Portrait-Box-Mouth

- Status: PASS818_DM1_V1_CHAMPION_PORTRAIT_BOX_EYE_LOCKED
- Gate: Graphics.dat item 562 init var G0048_ai_Graphic562_Box_Eye[4] = {11, 28, 12, 29}. The {X, Y, W, H} pixel-coordinate rectangle for the champion-portrait eye-overlay blit. CHAMDRAW.C F0294_CHAMPION_DrawChampionPortraitEye calls M519_F0020_MAIN_BlitToViewport(nativeBitmap, G0048, C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY, 18) to blit the eye graphic on top of the portrait when the champion is mid-action.
- Runtime assertion floor: 52 assertions in `tests/test_dm1_v1_champion_portrait_box_eye_pc34_compat.c`.
- Expected test output: `52/52 assertions passed`.

## ReDMCSB Anchors

- DATA.C:87
- DATA.C:426
- DATA.C:1100
- CHAMDRAW.C:928

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-816 (Graphics.dat init-table gates batches 1+2+3+4+5).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_portrait_box_eye_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:426/1095, CHAMDRAW.C:928 in next source refresh.

Manifest: `parity-evidence/verification/pass818_dm1_v1_champion_portrait_box_eye_pc34_compat/manifest.json`
