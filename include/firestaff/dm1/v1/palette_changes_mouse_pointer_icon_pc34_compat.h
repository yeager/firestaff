#ifndef FIRESTAFF_DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICON_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICON_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0044_auc_Graphic562_PaletteChanges_MousePointerIcon[16].
 *
 * G0044 is the 16-entry palette-change map for the mouse-pointer icon. PC 3.4 init = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 12, 4, 4, 4}. Read site: IO.C:164 (declaration) + IO.C:2140/2167/2176/2443 (F0129_VIDEO_BlitShrinkWithPaletteChanges with the icon palette).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICON_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesChangesMousePointerIconResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICON_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry4;
    int lastEntry4;
    int allValuesInRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
    int entry12Special;
} DM1_V1_PaletteChangesChangesMousePointerIconResultPc34;

const unsigned char *
dm1_v1_palette_changes_mouse_pointer_icon_table_pc34(void);

int
dm1_v1_palette_changes_mouse_pointer_icon_size_pc34(void);

int
dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(int palette_index);

int
dm1_v1_palette_changes_mouse_pointer_icon_run_pc34(
    DM1_V1_PaletteChangesChangesMousePointerIconResultPc34 *out);

#endif
