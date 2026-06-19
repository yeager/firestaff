#ifndef FIRESTAFF_DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0006_ai_Graphic562_Box_Entrance_DungeonView[4].
 *
 * G0006 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * ENTRANCE.C F0132_VIDEO_Blit to draw the dungeon-view backdrop
 * during the entrance sequence (door opening animation). Init
 * value (DATA.C:131 + DATA.C:551): { 0, 223, 3, 138 }.
 *
 * Read sites:
 * - ENTRANCE.C:178/181/184/187 F0132_VIDEO_Blit(
 *   G0296_puc_Bitmap_Viewport,
 *   L1394_ppuc_Bitmap_EntranceDoorAnimationSteps[9],
 *   G0006, 0, 0, C112_BYTE_WIDTH_VIEWPORT, C128_BYTE_WIDTH,
 *   CM1_COLOR_NO_TRANSPARENCY) — blit the entrance dungeon-view
 *   backdrop during the door-opening animation.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832 (Graphics.dat init-table gates batches 1+2+3+4+
 * 5+6+7+8+9+10+11). This gate is a non-mirror-candidate contract
 * for the G0006 entrance-dungeon-view box.
 */

#define DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxEntranceDungeonViewResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs223;
    int wIs3;
    int hIs138;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinViewportWidth;
    int withinBoxBounds;
} DM1_V1_BoxEntranceDungeonViewResultPc34;

const int *
dm1_v1_box_entrance_dungeon_view_table_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_size_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_get_pc34(int component, int *out_value);

int
dm1_v1_box_entrance_dungeon_view_x_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_y_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_w_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_h_pc34(void);

int
dm1_v1_box_entrance_dungeon_view_run_pc34(
    DM1_V1_BoxEntranceDungeonViewResultPc34 *out);

#endif