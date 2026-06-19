#ifndef FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_BOX_MOUTH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PORTRAIT_BOX_MOUTH_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0048_ai_Graphic562_Box_Eye[4].
 *
 * G0048 is the {X, Y, W, H} pixel-coordinate rectangle used by
 * CHAMDRAW.C to blit the champion-portrait eye graphic on top of
 * the portrait area (when the champion is mid-action / mid-talk).
 * PC 3.4 init (DATA.C:423 + DATA.C:1095): { 11, 28, 12, 29 }.
 *
 * Read site:
 * - CHAMDRAW.C:928 F0294_CHAMPION_DrawChampionPortraitEye — blit
 *   the eye graphic into G0048's box with byte width C016_BYTE_WIDTH
 *   and color C12_COLOR_DARKEST_GRAY, bit depth 18 (8bpp + 8 padding
 *   for EGA-style byte-aligned blit).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816 (Graphics.dat init-
 * table gates batches 1+2+3+4+5). This gate is a non-mirror-
 * candidate contract for the G0048 eye-overlay blit rectangle.
 */

typedef struct DM1_V1_ChampionPortraitBoxEyeResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[4];
    int tableMatchesDeclaration;
    int xIs11;
    int yIs28;
    int wIs12;
    int hIs29;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int withinReasonableBounds;
    int heightLargerThanWidth;
} DM1_V1_ChampionPortraitBoxEyeResultPc34;

const int *
dm1_v1_champion_portrait_box_eye_table_pc34(void);

int
dm1_v1_champion_portrait_box_eye_get_pc34(int component, int *out_value);

int
dm1_v1_champion_portrait_box_eye_x_pc34(void);

int
dm1_v1_champion_portrait_box_eye_y_pc34(void);

int
dm1_v1_champion_portrait_box_eye_w_pc34(void);

int
dm1_v1_champion_portrait_box_eye_h_pc34(void);

int
dm1_v1_champion_portrait_box_eye_run_pc34(
    DM1_V1_ChampionPortraitBoxEyeResultPc34 *out);

#endif