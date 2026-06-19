#ifndef FIRESTAFF_DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0029_auc_Graphic562_ChargeCountToTorchType[16].
 *
 * G0029 maps a torch's remaining charge count (0..15) to the
 * torch-icon type (0..3) used to draw that torch on the inventory
 * panel. The values {0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
 * 3, 3} mean: 0 charges -> type 0, 1-3 -> type 1, 4-7 -> type 2,
 * 8-15 -> type 3.
 *
 * Read site: OBJECT.C:178 F0486_OBJECT_DrawObjectIcon
 *   case C004_ICON_WEAPON_TORCH_UNLIT:
 *       if (((WEAPON*)L0006_ps_Junk)->Lit) {
 *           L0005_i_IconIndex +=
 *               G0029_auc_Graphic562_ChargeCountToTorchType
 *                   [((WEAPON*)L0006_ps_Junk)->ChargeCount];
 *       }
 *       break;
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index),
 * pass803 (ordered-cells). This gate is a non-mirror-candidate
 * contract for the G0029 torch-charge-count -> torch-icon-type
 * mapping.
 */

#define DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_PC34_COMPAT_SIZE 16
#define DM1_V1_TORCH_TYPE_COUNT 4

typedef struct DM1_V1_ChargeCountToTorchTypeResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_CHARGE_COUNT_TO_TORCH_TYPE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int tableIsMonotonic;
    int tableHas4DistinctValues;
    int firstEntry0;
    int lastEntry3;
    int allWithinRange0to3;
    int bucketBoundariesCorrect;
    int lookupFunctionInRange;
    int lookupOutOfRangeReturnsZero;
    int bucketBoundaries0183ToType0123Correct;
    int dispatchFunctionCorrect;
} DM1_V1_ChargeCountToTorchTypeResultPc34;

const int *
dm1_v1_charge_count_to_torch_type_table_pc34(void);

int
dm1_v1_charge_count_to_torch_type_size_pc34(void);

int
dm1_v1_charge_count_to_torch_type_pc34(int charge_count);

int
dm1_v1_charge_count_to_torch_type_bucket_pc34(int charge_count);

int
dm1_v1_charge_count_to_torch_type_first_count_for_type_pc34(int torch_type);

int
dm1_v1_charge_count_to_torch_type_last_count_for_type_pc34(int torch_type);

int
dm1_v1_charge_count_to_torch_type_run_pc34(
    DM1_V1_ChargeCountToTorchTypeResultPc34 *out);

#endif