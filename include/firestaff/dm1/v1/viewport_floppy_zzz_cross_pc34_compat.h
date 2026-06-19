#ifndef FIRESTAFF_DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0041_ai_Graphic562_Box_ViewportFloppyZzzCross[4].
 *
 * G0041 is the {X, Y, W, H} byte-coordinate sub-rectangle used by PANEL.C to fill the floppy-disk ZZZ-cross overlay. Init value {174, 218, 2, 12}.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

#define DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxViewportFloppyZzzCrossResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOXVIEWPORTFLOPPYZZZCROSS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs174;
    int yIs218;
    int wIs2;
    int hIs12;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxViewportFloppyZzzCrossResultPc34;

const int *
dm1_v1_viewport_floppy_zzz_cross_table_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_size_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_get_pc34(int component, int *out_value);

int
dm1_v1_viewport_floppy_zzz_cross_x_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_y_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_w_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_h_pc34(void);

int
dm1_v1_viewport_floppy_zzz_cross_run_pc34(
    DM1_V1_BoxViewportFloppyZzzCrossResultPc34 *out);

#endif
