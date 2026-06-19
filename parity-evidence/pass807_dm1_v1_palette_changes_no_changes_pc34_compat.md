# pass807 DM1 V1 Palette-Changes-No-Changes

- Status: PASS807_DM1_V1_PALETTE_CHANGES_NO_CHANGES_LOCKED
- Gate: Graphics.dat item 562 init var G0017_auc_Graphic562_PaletteChanges_NoChanges[16]. PC 3.4 init = {0, 1, ..., 15} (identity permutation). Atari init = {0, 10, 20, ..., 150} (10-step ramp). Passed as the PaletteChanges argument to F0129_VIDEO_BlitShrinkWithPaletteChanges when the caller wants no palette remapping. Read sites: ACTIDRAW.C:160 (creature action icon), BLTSHRNK.C:530 (identity-path guard), DUNVIEW.C:4518/5973 (explosion/projectile aspect), STARTUP2.C:822 (G0075 init).
- Runtime assertion floor: 98 assertions in `tests/test_dm1_v1_palette_changes_no_changes_pc34_compat.c`.
- Expected test output: `98/98 assertions passed`.

## ReDMCSB Anchors

- DATA.C:23
- DATA.C:136
- DATA.C:590
- ACTIDRAW.C:160
- BLTSHRNK.C:530
- DUNVIEW.C:4518/5973
- STARTUP2.C:822

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + chest).
- Not pass798-806 (Graphics.dat init-table gates batches 1+2).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_palette_changes_no_changes_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass807_dm1_v1_palette_changes_no_changes_pc34_compat/manifest.json`
