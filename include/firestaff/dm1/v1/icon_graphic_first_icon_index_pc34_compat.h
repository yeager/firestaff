#ifndef FIRESTAFF_DM1_V1_ICON_GRAPHIC_FIRST_ICON_INDEX_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ICON_GRAPHIC_FIRST_ICON_INDEX_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0026_ai_Graphic562_IconGraphicFirstIconIndex[7].
 *
 * G0026 holds the *first icon index* in each of the seven 32-icon
 * graphic blocks (#42..#48) that OBJECT.C walks when resolving an
 * icon index to a graphic block + within-block offset.
 *
 * Init values per DATA.C:253-260 + DATA.C:911-918:
 *   G0026[0] =   0 (first icon in graphic #42, indices 0..31)
 *   G0026[1] =  32 (first icon in graphic #43, indices 32..63)
 *   G0026[2] =  64 (first icon in graphic #44, indices 64..95)
 *   G0026[3] =  96 (first icon in graphic #45, indices 96..127)
 *   G0026[4] = 128 (first icon in graphic #46, indices 128..159)
 *   G0026[5] = 160 (first icon in graphic #47, indices 160..191)
 *   G0026[6] = 192 (first icon in graphic #48, indices 192..223)
 *
 * Read sites:
 * - OBJECT.C:312-319 F0489_MEMORY_GetNativeBitmapOrGraphicIcon_Loop:
 *       for (c = 0; c < 7; ++c)
 *           if (G0026[c] > IconIndex) break;
 *       IconGraphicIndex = --c;
 *       IconIndex -= G0026[IconGraphicIndex];
 *       Bitmap = GetNativeBitmapOrGraphic(C042_GRAPHIC_OBJECT_ICONS + IconGraphicIndex);
 * - OBJECT.C:455-467 same loop, with the slot-box IconIndex path
 *   (P0048 = G0030_as_Graphic562_SlotBoxes[P0047_ui_SlotBoxIndex].IconIndex).
 *
 * The table is monotonically non-decreasing by 32 each step, so the
 * 32-icon block size is the actual contract here.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-796 (champion-panel/leader/mirror). This gate is a
 * non-mirror-candidate contract for the OBJECT.C icon resolution path.
 */

typedef struct DM1_V1_IconGraphicFirstIconIndexResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[7];
    int tableSize;
    int tableMatchesDeclaration;
    int firstBlockStartZero;
    int lastBlockStart192;
    int monotonicIncreasing32;
    int allWithinIconRange;
    int lookupGraph42_Index0;
    int lookupGraph43_Index32;
    int lookupGraph44_Index64;
    int lookupGraph45_Index96;
    int lookupGraph46_Index128;
    int lookupGraph47_Index160;
    int lookupGraph48_Index192;
    int outOfRangeReturnsMinusOne;
    int declarationMatchesInit;
} DM1_V1_IconGraphicFirstIconIndexResultPc34;

const int *
dm1_v1_icon_graphic_first_icon_index_table_pc34(void);

int
dm1_v1_icon_graphic_first_icon_index_size_pc34(void);

int
dm1_v1_icon_graphic_first_icon_index_pc34(int index);

int
dm1_v1_icon_graphic_first_icon_index_block_size_pc34(void);

int
dm1_v1_icon_graphic_first_icon_index_first_graph_pc34(void);

int
dm1_v1_icon_graphic_first_icon_index_graph_count_pc34(void);

int
dm1_v1_icon_graphic_first_icon_index_resolve_pc34(
    int icon_index,
    int *out_graph_index,
    int *out_within_block_index);

int
dm1_v1_icon_graphic_first_icon_index_run_pc34(
    DM1_V1_IconGraphicFirstIconIndexResultPc34 *out);

#endif
