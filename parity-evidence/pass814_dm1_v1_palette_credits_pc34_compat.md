# pass814 DM1 V1 Palette-Credits

- Status: PASS814_DM1_V1_PALETTE_CREDITS_LOCKED
- Gate: Graphics.dat item 562 init var G0019_aui_Graphic562_Palette_Credits[16] = { 0x009, 0x0AA, 0xFF6, 0x840, 0xFF8, 0x000, 0x080, 0xA00, 0xC84, 0xFFA, 0xF84, 0xFC0, 0xFA0, 0x000, 0x620, 0xFFC }. The 16-color palette used for the Credits screen fade (12-bit RGB, 4 bits per channel). Read sites: ENDGAME.C:680 + ENTRANCE.C:1061 F0436_STARTEND_FadeToPalette(G0019).
- Runtime assertion floor: 103 assertions in `tests/test_dm1_v1_palette_credits_pc34_compat.c`.
- Expected test output: `103/103 assertions passed`.

## ReDMCSB Anchors

- DATA.C:25
- DATA.C:210
- DATA.C:789
- ENDGAME.C:680
- ENTRANCE.C:1061

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-813 (Graphics.dat init-table gates batches 1+2+3+4).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_palette_credits_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass814_dm1_v1_palette_credits_pc34_compat/manifest.json`
