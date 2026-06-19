#ifndef FIRESTAFF_DM1_V1_BOX_TITLE_STRIKES_BACK_DESTINATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_TITLE_STRIKES_BACK_DESTINATION_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0003_ai_Graphic562_Box_Title_StrikesBack_Destination[4].
 *
 * G0003 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * TITLE.C F0132_VIDEO_Blit to draw the "Dungeon Master II: The
 * Legend of Skullkeep - Strikes Back" title screen destination
 * rectangle. Init value (DATA.C:125 + DATA.C:545): { 0, 319, 118,
 * 174 }.
 *
 * Read sites:
 * - TITLE.C:233/236 F0132_VIDEO_Blit(L1389_puc_Bitmap_Master_"
 *   StrikesBack, G0348_Bitmap_Screen, G0003, 0, 0, C160_BYTE_WIDTH_
 *   SCREEN, C160_BYTE_WIDTH_SCREEN, C00_COLOR_BLACK, 57, C200_HEIGHT_
 *   SCREEN) — blit the "Strikes Back" title graphic to the
 *   destination rectangle.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832/833/834 (Graphics.dat init-table gates batches
 * 1+2+3+4+5+6+7+8+9+10+11+12+13). This gate is a non-mirror-
 * candidate contract for the G0003 Strikes-Back title destination
 * box.
 */

#define DM1_V1_BOX_TITLE_STRIKES_BACK_DESTINATION_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxTitleStrikesBackDestinationResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_TITLE_STRIKES_BACK_DESTINATION_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs319;
    int wIs118;
    int hIs174;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxTitleStrikesBackDestinationResultPc34;

const int *
dm1_v1_box_title_strikes_back_destination_table_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_size_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_get_pc34(int component, int *out_value);

int
dm1_v1_box_title_strikes_back_destination_x_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_y_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_w_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_h_pc34(void);

int
dm1_v1_box_title_strikes_back_destination_run_pc34(
    DM1_V1_BoxTitleStrikesBackDestinationResultPc34 *out);

#endif