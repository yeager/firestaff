#ifndef FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0047_auc_Graphic562_Box_ChampionPortrait[4].
 *
 * G0047 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * REVIVE.C F0132_VIDEO_Blit to extract a single champion portrait
 * (32x32 pixels = 32 bytes per row) from the C026_GRAPHIC_CHAMPION_
 * PORTRAITS source bitmap. The X/Y are byte offsets into the
 * portrait-source row (always 0), and W/H are byte counts (31 bytes
 * wide x 28 bytes tall — the inner portrait area excluding the
 * decorative border).
 *
 * Init value (DATA.C:424 + DATA.C:1098): { 0, 31, 0, 28 }.
 *
 * Read sites:
 * - REVIVE.C:142 F0132_VIDEO_Blit for the champion's permanent
 *   Portrait bitmap (M027_PORTRAIT_X/Y).
 * - REVIVE.C:146 F0132_VIDEO_Blit for the temporary Portrait
 *   ChipMemory buffer.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818 (Graphics.dat
 * init-table gates batches 1+2+3+4+5+6). This gate is a non-mirror-
 * candidate contract for the G0047 portrait-extraction rectangle.
 */

#define DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxChampionPortraitResultPc34 {
    int accepted;
    int assertionCount;
    unsigned char tableEntries[DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs0;
    int yIs31;
    int wIs0;
    int hIs28;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int byteAligned;
    int withinByteRange;
} DM1_V1_BoxChampionPortraitResultPc34;

const unsigned char *
dm1_v1_box_champion_portrait_table_pc34(void);

int
dm1_v1_box_champion_portrait_size_pc34(void);

int
dm1_v1_box_champion_portrait_get_pc34(int component, int *out_value);

int
dm1_v1_box_champion_portrait_x_pc34(void);

int
dm1_v1_box_champion_portrait_y_pc34(void);

int
dm1_v1_box_champion_portrait_w_pc34(void);

int
dm1_v1_box_champion_portrait_h_pc34(void);

int
dm1_v1_box_champion_portrait_run_pc34(
    DM1_V1_BoxChampionPortraitResultPc34 *out);

#endif