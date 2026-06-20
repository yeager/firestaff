#ifndef FIRESTAFF_DM1_V1_BARGRAPHBYTEOFFSETS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BARGRAPHBYTEOFFSETS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0056_aaui_Graphic562_BarGraphByteOffsets[4][3].
 *
 * G0056 is the 4-champion × 3-graph (health/mana/stamina) byte-offset
 * table used by the status-panel bar-graph renderer to find the
 * per-byte health/mana/stamina value to mask with G0055. PC 3.4 EN
 * init = {
 *   C0: { 16, 24, 24},
 *   C1: { 56, 56, 64},
 *   C2: { 88, 88, 96},
 *   C3: {120,128,128}
 * }. Read site: CHAMDRAW.C:204 (lea G0056, A1) + 207-208 (inline
 * comment).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-859.
 */

#define DM1_V1_BAR_GRAPH_BYTE_OFFSETS_PC34_COMPAT_CHAMPION_COUNT 4
#define DM1_V1_BAR_GRAPH_BYTE_OFFSETS_PC34_COMPAT_GRAPH_COUNT 3
#define DM1_V1_BAR_GRAPH_BYTE_OFFSETS_PC34_COMPAT_SIZE 12

typedef struct DM1_V1_BarGraphByteOffsetsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BAR_GRAPH_BYTE_OFFSETS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int champion0HealthOffset16;
    int champion1HealthOffset56;
    int champion2HealthOffset88;
    int champion3HealthOffset120;
    int allOffsetsNonNegative;
    int allOffsetsInByteRange;
    int monotonicPerChampion;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_BarGraphByteOffsetsResultPc34;

const unsigned int *
dm1_v1_bar_graph_byte_offsets_table_pc34(void);

int
dm1_v1_bar_graph_byte_offsets_size_pc34(void);

int
dm1_v1_bar_graph_byte_offsets_get_pc34(int champion_index, int graph_index);

int
dm1_v1_bar_graph_byte_offsets_run_pc34(
    DM1_V1_BarGraphByteOffsetsResultPc34 *out);

#endif