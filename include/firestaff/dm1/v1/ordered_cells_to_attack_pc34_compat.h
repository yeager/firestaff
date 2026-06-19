#ifndef FIRESTAFF_DM1_V1_ORDERED_CELLS_TO_ATTACK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ORDERED_CELLS_TO_ATTACK_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0023_aac_Graphic562_OrderedCellsToAttack[8][4].
 *
 * G0023 is the creature-AI attack-direction priority table: each of
 * the 8 rows maps to (attack-direction, attacker-position) tuple,
 * and each row holds 4 directional offsets to try in order when a
 * creature is attacking a square it cannot reach directly.
 *
 * Row layout (DATA.C:234-243 + DATA.C:878-887, PC 3.4 + post-1.3
 * Atari init):
 *   0: South from NW or SW    -> {0, 1, 3, 2}
 *   1: South from NE or SE    -> {1, 0, 2, 3}
 *   2: West from NW or NE     -> {1, 2, 0, 3}
 *   3: West from SE or SW     -> {2, 1, 3, 0}
 *   4: North from NW or SW    -> {3, 2, 0, 1}
 *   5: North from SE or NE    -> {2, 3, 1, 0}
 *   6: East from NW or NE     -> {0, 3, 1, 2}
 *   7: East from SE or SW     -> {3, 0, 2, 1}
 *
 * Read site: PROJEXPL.C:1302 F0229_GROUP_SetOrderedCellsToAttack
 *   L0557_ui_OrderedCellsToAttackIndex = (F0228_GROUP_GetDirectionsWhere
 *       DestinationIsVisibleFromSource(...) << 1) & 0x0002 + ...;
 *   F0007_MAIN_CopyBytes(G0023[Index], P0487_pc_OrderedCellsToAttack,
 *       M543_BYTE_COUNT_INT(4));
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-799 (champion-panel/leader/mirror + auto-chest +
 * chest-open-stack-split), pass798 (icon-graphic), pass800
 * (slot-boxes), pass801 (light-power), pass802 (palette-index).
 * This gate is a non-mirror-candidate contract for the G0023
 * creature-AI attack-direction priority table.
 */

#define DM1_V1_ORDERED_CELLS_TO_ATTACK_ROWS_PC34_COMPAT 8
#define DM1_V1_ORDERED_CELLS_TO_ATTACK_COLS_PC34_COMPAT 4

typedef struct DM1_V1_OrderedCellsToAttackResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_ORDERED_CELLS_TO_ATTACK_ROWS_PC34_COMPAT
                     * DM1_V1_ORDERED_CELLS_TO_ATTACK_COLS_PC34_COMPAT];
    int tableRows;
    int tableCols;
    int tableMatchesDeclaration;
    int rowCountIs8;
    int colCountIs4;
    int allValuesInRange0to3;
    int eachRowPermutationOf4Directions;
    int all8RowsDistinct;
    int dispatchFunctionCorrect;
    int dispatchOutOfRangeRejects;
    int dispatchNullSafe;
} DM1_V1_OrderedCellsToAttackResultPc34;

const int *
dm1_v1_ordered_cells_to_attack_table_pc34(void);

int
dm1_v1_ordered_cells_to_attack_row_count_pc34(void);

int
dm1_v1_ordered_cells_to_attack_col_count_pc34(void);

int
dm1_v1_ordered_cells_to_attack_pc34(int row, int col);

int
dm1_v1_ordered_cells_to_attack_row_is_permutation_pc34(int row);

int
dm1_v1_ordered_cells_to_attack_dispatch_pc34(
    int row,
    int *out_first,
    int *out_second,
    int *out_third,
    int *out_fourth);

int
dm1_v1_ordered_cells_to_attack_run_pc34(
    DM1_V1_OrderedCellsToAttackResultPc34 *out);

#endif