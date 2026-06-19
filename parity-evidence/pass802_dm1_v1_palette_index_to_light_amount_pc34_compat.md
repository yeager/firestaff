# pass802 DM1 V1 Palette-Index-To-Light-Amount

- Status: PASS802_DM1_V1_PALETTE_INDEX_TO_LIGHT_AMOUNT_LOCKED
- Gate: Graphics.dat item 562 init var G0040_ai_Graphic562_PaletteIndexToLightAmount[6] = {99, 75, 50, 25, 1, 0}. PANEL.C F0337_INVENTORY_SetDungeonViewPalette walks the table top-down: `while (*p++ > TotalLightAmount) PaletteIndex++`. Result: palette 0 (brightest) for TotalLightAmount >= 99, palette 5 (darkest) for TotalLightAmount <= 0, with thresholds at 99/75/50/25/1 boundaries.
- Runtime assertion floor: 82 assertions in `tests/test_dm1_v1_palette_index_to_light_amount_pc34_compat.c`.
- Expected test output: `82/82 assertions passed`.

## ReDMCSB Anchors

- DATA.C:46
- DATA.C:360
- DATA.C:1089
- PANEL.C:419-423

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + auto-chest + chest-open-stack-split).
- Not pass798 (icon-graphic-first-icon-index).
- Not pass800 (slot-boxes).
- Not pass801 (light-power-to-light-amount).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_palette_index_to_light_amount_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass802_dm1_v1_palette_index_to_light_amount_pc34_compat/manifest.json`
