#ifndef FIRESTAFF_DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0004_ai_Graphic562_Box_Title_StrikesBack_Source[4].
 *
 * G0004 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * TITLE.C F0132_VIDEO_Blit to draw the "Dungeon Master II: The
 * Legend of Skullkeep - Strikes Back" title source rectangle
 * (the source bitmap area to copy FROM). Init value
 * (DATA.C:127 + DATA.C:547): { 0, 319, 0, 56 }.
 *
 * Read sites:
 * - TITLE.C:134/137/335/338 F0132_VIDEO_Blit(L1384_puc_Bitmap_
 *   Title, L1389_puc_Bitmap_Master_StrikesBack, G0004, 0, 80,
 *   C160_BYTE_WIDTH_SCREEN, C160_BYTE_WIDTH_SCREEN, CM1_COLOR_NO_
 *   TRANSPARENCY) — blit the Strikes-Back title source from
 *   G0004's source rect.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834/835 (Graphics.dat init-table gates
 * batches 1+2+3+4+5+6+7+8+9+10+11+12+13+14). This gate is a
 * non-mirror-candidate contract for the G0004 Strikes-Back title
 * source box.
 */

#define DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxTitleStrikesBackSourceResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_TITLE_STRIKES_BACK_SOURCE_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs319;
    int wIs0;
    int hIs56;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxTitleStrikesBackSourceResultPc34;

const int *
dm1_v1_box_title_strikes_back_source_table_pc34(void);

int
dm1_v1_box_title_strikes_back_source_size_pc34(void);

int
dm1_v1_box_title_strikes_back_source_get_pc34(int component, int *out_value);

int
dm1_v1_box_title_strikes_back_source_x_pc34(void);

int
dm1_v1_box_title_strikes_back_source_y_pc34(void);

int
dm1_v1_box_title_strikes_back_source_w_pc34(void);

int
dm1_v1_box_title_strikes_back_source_h_pc34(void);

int
dm1_v1_box_title_strikes_back_source_run_pc34(
    DM1_V1_BoxTitleStrikesBackSourceResultPc34 *out);

#endif