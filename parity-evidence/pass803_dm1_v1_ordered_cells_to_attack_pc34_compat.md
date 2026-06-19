# pass803 DM1 V1 Ordered-Cells-To-Attack

- Status: PASS803_DM1_V1_ORDERED_CELLS_TO_ATTACK_LOCKED
- Gate: Graphics.dat item 562 init var G0023_aac_Graphic562_OrderedCellsToAttack[8][4] = 8 rows of 4 directions each. Each row is a permutation of {0, 1, 2, 3} (SOUTH/EAST/NORTH/WEST) representing the attack-direction priority list for one (attack-direction, attacker-position) tuple. Read site: PROJEXPL.C:1302 F0229_GROUP_SetOrderedCellsToAttack copies the row to a 4-byte buffer.
- Runtime assertion floor: 222 assertions in `tests/test_dm1_v1_ordered_cells_to_attack_pc34_compat.c`.
- Expected test output: `222/222 assertions passed`.

## ReDMCSB Anchors

- DATA.C:29
- DATA.C:234-243
- DATA.C:878-887
- PROJEXPL.C:1302
- DEFS.H:7640-7660

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + auto-chest + chest-open-stack-split).
- Not pass798 (icon-graphic-first-icon-index).
- Not pass800 (slot-boxes).
- Not pass801 (light-power-to-light-amount).
- Not pass802 (palette-index-to-light-amount).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_ordered_cells_to_attack_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass803_dm1_v1_ordered_cells_to_attack_pc34_compat/manifest.json`
