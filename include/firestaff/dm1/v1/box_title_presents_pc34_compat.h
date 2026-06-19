#ifndef FIRESTAFF_DM1_V1_BOX_TITLE_PRESENTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_TITLE_PRESENTS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0005_ai_Graphic562_Box_Title_TitlePresents_Source[4].
 *
 * G0005 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * TITLE.C F0132_VIDEO_Blit to draw the "Dungeon Master II: The
 * Legend of Skullkeep - Title Presents" title source rectangle
 * (the source bitmap area to copy FROM). Init value
 * (DATA.C:129 + DATA.C:549): { 0, 319, 90, 105 }.
 *
 * Read sites:
 * - TITLE.C:126/129/321/324 F0132_VIDEO_Blit(L1384_puc_Bitmap_
 *   Title, L1389_puc_Bitmap_Master_TitlePresents, G0005, 0, 80,
 *   C160_BYTE_WIDTH_SCREEN, C160_BYTE_WIDTH_SCREEN, CM1_COLOR_NO_
 *   TRANSPARENCY) — blit the Title-Presents source from
 *   G0005's source rect.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835 (Graphics.dat init-table gates
 * batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14). This gate is a
 * non-mirror-candidate contract for the G0005 Title-Presents
 * source box.
 */

#define DM1_V1_BOX_TITLE_PRESENTS_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxTitlePresentsResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_TITLE_PRESENTS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs319;
    int wIs90;
    int hIs105;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxTitlePresentsResultPc34;

const int *
dm1_v1_box_title_presents_table_pc34(void);

int
dm1_v1_box_title_presents_size_pc34(void);

int
dm1_v1_box_title_presents_get_pc34(int component, int *out_value);

int
dm1_v1_box_title_presents_x_pc34(void);

int
dm1_v1_box_title_presents_y_pc34(void);

int
dm1_v1_box_title_presents_w_pc34(void);

int
dm1_v1_box_title_presents_h_pc34(void);

int
dm1_v1_box_title_presents_run_pc34(
    DM1_V1_BoxTitlePresentsResultPc34 *out);

#endif