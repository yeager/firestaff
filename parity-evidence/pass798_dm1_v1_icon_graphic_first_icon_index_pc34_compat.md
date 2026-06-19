# pass798 DM1 V1 Icon-Graphic-First-Icon-Index

- Status: PASS798_DM1_V1_ICON_GRAPHIC_FIRST_ICON_INDEX_LOCKED
- Gate: Graphics.dat item 562 init var G0026_ai_Graphic562_IconGraphicFirstIconIndex[7] = {0, 32, 64, 96, 128, 160, 192}. OBJECT.C F0489 walks the table in a 7-iteration loop (OBJECT.C:312-319 and OBJECT.C:455-467) to find the icon-graphic block + within-block offset for each icon index. Stride is 32 (one graphic block); 7 blocks cover 224 icons.
- Runtime assertion floor: 86 assertions in `tests/test_dm1_v1_icon_graphic_first_icon_index_pc34_compat.c`.
- Expected test output: `86/86 assertions passed`.

## ReDMCSB Anchors

- DATA.C:32
- DATA.C:253-260
- OBJECT.C:312-319
- OBJECT.C:455-467
- OBJECT.C:521

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-796 (champion-panel/leader/mirror).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_icon_graphic_first_icon_index_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass798_dm1_v1_icon_graphic_first_icon_index_pc34_compat/manifest.json`
