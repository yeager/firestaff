#ifndef FIRESTAFF_DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICONSHADOW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICONSHADOW_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0045_auc_Graphic562_PaletteChanges_MousePointerIconShadow[16].
 *
 * G0045 is the 16-entry palette-change map for the mouse-pointer icon shadow. PC 3.4 init = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0}. Read site: IO.C:165 (declaration) + IO.C:1708/1711/2041 (F1004/F0129/F0663 with the shadow palette).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICONSHADOW_PC34_COMPAT_SIZE 16

typedef struct DM1_V1_PaletteChangesChangesMousePointerIconShadowResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_PALETTECHANGESCHANGESMOUSEPOINTERICONSHADOW_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstEntry0;
    int lastEntry0;
    int allValuesInRange;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
    int entry12Special;
} DM1_V1_PaletteChangesChangesMousePointerIconShadowResultPc34;

const unsigned char *
dm1_v1_palette_changes_mouse_pointer_icon_shadow_table_pc34(void);

int
dm1_v1_palette_changes_mouse_pointer_icon_shadow_size_pc34(void);

int
dm1_v1_palette_changes_mouse_pointer_icon_shadow_get_pc34(int palette_index);

int
dm1_v1_palette_changes_mouse_pointer_icon_shadow_run_pc34(
    DM1_V1_PaletteChangesChangesMousePointerIconShadowResultPc34 *out);

#endif
