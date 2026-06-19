# pass819 DM1 V1 Champion-Portrait-Box-Mouth

- Status: FAILED_PASS819_DM1_V1_BOX_CHAMPION_PORTRAIT_LOCKED
- Gate: Graphics.dat item 562 init var G0048_ai_Graphic562_Box_ChampionPortrait[4] = {0, 31, 0, 28}. The {X, Y, W, H} pixel-coordinate rectangle for the champion-portrait portrait-extraction blit. CHAMDRAW.C F0132_VIDEO_Blit calls M519_F0020_MAIN_BlitToViewport(nativeBitmap, G0048, C016_BYTE_WIDTH, C12_COLOR_DARKEST_GRAY, 18) to blit the portrait source bitmap on top of the portrait when the champion is mid-action.
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
- Not pass798-816 (Graphics.dat init-table gates batches 1+2+3+4+5).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_champion_portrait_box_champion_portrait_pc34_compat`: rc=0

## TODO

- Anchor drift note: re-pin DATA.C:424/1095, REVIVE.C:142/146 in next source refresh.

Manifest: `parity-evidence/verification/pass819_champion_portrait_box_champion_portrait_pc34_compat/manifest.json`
