# pass804 DM1 V1 Charge-Count-To-Torch-Type

- Status: PASS804_DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_LOCKED
- Gate: Graphics.dat item 562 init var G0029_auc_Graphic562_ChargeCountToTorchType[16] = {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3}. Bucket design: 0 charges -> type 0, 1..3 charges -> type 1, 4..7 charges -> type 2, 8..15 charges -> type 3. OBJECT.C:178 F0486_OBJECT_DrawObjectIcon reads this table to pick the torch-icon variant when drawing a lit torch weapon.
- Runtime assertion floor: 144 assertions in `tests/test_dm1_v1_charge_count_to_torch_type_pc34_compat.c`.
- Expected test output: `144/144 assertions passed`.

## ReDMCSB Anchors

- DATA.C:35
- DATA.C:263
- DATA.C:926
- OBJECT.C:178

## Non-Overlap

- Not pass784-790.
- Not pass791 (champion-panel ammo-compat).
- Not pass792 (steal-from-slot-indices).
- Not pass793-799 (champion-panel/leader/mirror + auto-chest + chest-open-stack-split).
- Not pass798 (icon-graphic-first-icon-index).
- Not pass800 (slot-boxes).
- Not pass801 (light-power-to-light-amount).
- Not pass802 (palette-index-to-light-amount).
- Not pass803 (ordered-cells-to-attack).

## Verification

- `/Users/bosse/.openclaw/workspace-main/build/test_dm1_v1_charge_count_to_torch_type_pc34_compat`: rc=0

Manifest: `parity-evidence/verification/pass804_dm1_v1_charge_count_to_torch_type_pc34_compat/manifest.json`
