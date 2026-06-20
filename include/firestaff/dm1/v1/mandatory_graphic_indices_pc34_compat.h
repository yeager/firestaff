#ifndef FIRESTAFF_DM1_V1_MANDATORYGRAPHICINDICES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MANDATORYGRAPHICINDICES_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0018_ai_Graphic562_MandatoryGraphicIndices[M531_MANDATORY_GRAPHIC_COUNT].
 *
 * G0018 is the list of graphics that must always be loaded.
 * Read sites: DUNVIEW.C:2350/2353/2356/2454 (F0007_MAIN_CopyBytes
 * into L0077_pi_GraphicIndices), DUNVIEW.C:2359/2455 (sets
 * AL0075_ui_GraphicCount to M531_MANDATORY_GRAPHIC_COUNT). PC 3.4
 * EN init block: DATA.C:137-202 (63 visible entries after the
 * #ifdef MEDIA388_G20E_G21E guard).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-851.
 */

#define DM1_V1_MANDATORY_GRAPHIC_INDICES_PC34_COMPAT_COUNT 70

typedef struct DM1_V1_MandatoryGraphicIndicesResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_MANDATORY_GRAPHIC_INDICES_PC34_COMPAT_COUNT];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntryStatusBox;
    int lastEntryBorderPartySpellshield;
    int allValuesNonNegative;
    int allValuesDistinct;
    int firstIconBase42;
    int lastFloorPitInvisibleD0C;
    int fieldMaskStride;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_MandatoryGraphicIndicesResultPc34;

const int *
dm1_v1_mandatory_graphic_indices_table_pc34(void);

int
dm1_v1_mandatory_graphic_indices_size_pc34(void);

int
dm1_v1_mandatory_graphic_indices_get_pc34(int index);

int
dm1_v1_mandatory_graphic_indices_first_pc34(void);

int
dm1_v1_mandatory_graphic_indices_last_pc34(void);

int
dm1_v1_mandatory_graphic_indices_run_pc34(
    DM1_V1_MandatoryGraphicIndicesResultPc34 *out);

#endif