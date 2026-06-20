#ifndef FIRESTAFF_DM1_V1_BARGRAPHMASKS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BARGRAPHMASKS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0055_aaaui_Graphic562_BarGraphMasks[4][3][2].
 *
 * G0055 is the 4-champion × 3-graph (health/mana/stamina) × 2-mask
 * bitmask table used by the status-panel bar-graph renderer.
 * Each bar-graph is rendered by combining two 16-bit masks per
 * champion (4 champions × 3 bars × 2 masks = 24 uint16_t entries).
 * PC 3.4 EN init = {
 *   C0: {{0x0003, 0xC000}, {0x0780, 0x0000}, {0x000F, 0x0000}},
 *   C1: {{0x1E00, 0x0000}, {0x003C, 0x0000}, {0x7800, 0x0000}},
 *   C2: {{0x00F0, 0x0000}, {0x0001, 0xE000}, {0x03C0, 0x0000}},
 *   C3: {{0x0007, 0x8000}, {0x0F00, 0x0000}, {0x001E, 0x0000}}
 * }. Read site: CHAMDRAW.C:204/207/208 (the bar-graph mask LEA + comment).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-859.
 */

#define DM1_V1_BAR_GRAPH_MASKS_PC34_COMPAT_CHAMPION_COUNT 4
#define DM1_V1_BAR_GRAPH_MASKS_PC34_COMPAT_GRAPH_COUNT 3
#define DM1_V1_BAR_GRAPH_MASKS_PC34_COMPAT_MASK_PAIR 2
#define DM1_V1_BAR_GRAPH_MASKS_PC34_COMPAT_SIZE 24

typedef struct DM1_V1_BarGraphMasksResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BAR_GRAPH_MASKS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int champion0HealthMaskPair0003C000;
    int champion1HealthMaskPair1E000000;
    int champion2HealthMaskPair00F00000;
    int champion3HealthMaskPair00078000;
    int allMasksNonZero;
    int allMasksInUint16Range;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BarGraphMasksResultPc34;

const unsigned int *
dm1_v1_bar_graph_masks_table_pc34(void);

int
dm1_v1_bar_graph_masks_size_pc34(void);

int
dm1_v1_bar_graph_masks_get_pc34(int champion_index, int graph_index, int mask_index);

int
dm1_v1_bar_graph_masks_run_pc34(
    DM1_V1_BarGraphMasksResultPc34 *out);

#endif