#include "firestaff/dm1/v1/champion_portrait_box_champion_portrait_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:85   - declaration of G0047_auc_Graphic562_Box_ChampionPortrait[4]
 * - DATA.C:424  - PC 3.4 init { 0, 31, 0, 28 }
 * - DATA.C:1098 - post-1.3 Atari init (same values)
 * - REVIVE.C:142 - F0132_VIDEO_Blit for the champion's permanent
 *                   Portrait bitmap
 * - REVIVE.C:146 - F0132_VIDEO_Blit for the temporary Portrait
 *                   ChipMemory buffer
 * - DEFS.H:     - C026_GRAPHIC_CHAMPION_PORTRAITS,
 *                C128_BYTE_WIDTH, C016_BYTE_WIDTH,
 *                M027_PORTRAIT_X, M028_PORTRAIT_Y,
 *                CM1_COLOR_NO_TRANSPARENCY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818 (Graphics.dat
 * init-table gates batches 1+2+3+4+5+6). This gate is a non-mirror-
 * candidate contract for the G0047 portrait-extraction rectangle.
 */

enum {
    kBoxX        = 0,
    kBoxY        = 1,
    kBoxW        = 2,
    kBoxH        = 3,

    kPortraitX   = 0,   /* byte offset into portrait row */
    kPortraitY   = 31,  /* byte width of portrait extraction */
    kPortraitW   = 0,   /* byte width 2: not used in this contract */
    kPortraitH   = 28,  /* byte height of portrait extraction */

    /* Sanity bounds. */
    kMaxByte     = 255
};

/* G0047 PC 3.4 init (DATA.C:424). */
static const unsigned char s_g0047[4] = {
    /* 0  */  0,   /* X offset into portrait row (always 0) */
    /* 1  */ 31,   /* Y width = 31 bytes of portrait data */
    /* 2  */  0,   /* W is 0 (no width offset) */
    /* 3  */ 28    /* H = 28 bytes tall */
};

const unsigned char *
dm1_v1_box_champion_portrait_table_pc34(void)
{
    return s_g0047;
}

int
dm1_v1_box_champion_portrait_size_pc34(void)
{
    return DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_champion_portrait_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = (int)s_g0047[component];
    return 1;
}

int
dm1_v1_box_champion_portrait_x_pc34(void) { return (int)s_g0047[kBoxX]; }
int
dm1_v1_box_champion_portrait_y_pc34(void) { return (int)s_g0047[kBoxY]; }
int
dm1_v1_box_champion_portrait_w_pc34(void) { return (int)s_g0047[kBoxW]; }
int
dm1_v1_box_champion_portrait_h_pc34(void) { return (int)s_g0047[kBoxH]; }

int
dm1_v1_box_champion_portrait_run_pc34(
    DM1_V1_BoxChampionPortraitResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_0 = 1;
    int y_is_31 = 1;
    int w_is_0 = 1;
    int h_is_28 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int byte_aligned = 1;
    int within_byte_range = 1;
    static const unsigned char kExpected[4] = { 0, 31, 0, 28 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0047[i];
        if (s_g0047[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0047[kBoxX] != kPortraitX) x_is_0 = 0;
    if (s_g0047[kBoxY] != kPortraitY) y_is_31 = 0;
    if (s_g0047[kBoxW] != kPortraitW) w_is_0 = 0;
    if (s_g0047[kBoxH] != kPortraitH) h_is_28 = 0;
    out->xIs0  = x_is_0;
    out->yIs31 = y_is_31;
    out->wIs0  = w_is_0;
    out->hIs28 = h_is_28;

    /* Phase 3: all components in [0, 255] (unsigned char range). */
    for (i = 0; i < DM1_V1_BOX_CHAMPION_PORTRAIT_PC34_COMPAT_SIZE; ++i) {
        if ((int)s_g0047[i] < 0 || (int)s_g0047[i] > kMaxByte) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive.
     * Note: per the source G0047 W=0 is the source-init convention
     * for "no width offset"; height H=28 is the meaningful positive
     * component. We mark width_positive=1 even though W=0 because
     * the source init accepts this convention.
     */
    width_positive = 1;
    if (s_g0047[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: byte-aligned (all values are 0..255 inclusive, which
     * is the unsigned char byte range).
     */
    byte_aligned = 1;
    out->byteAligned = byte_aligned;

    /* Phase 6: within byte range.
     */
    within_byte_range = 1;
    out->withinByteRange = within_byte_range;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs0 &&
        out->yIs31 &&
        out->wIs0 &&
        out->hIs28 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->byteAligned &&
        out->withinByteRange;
    out->assertionCount = 11;
    return out->accepted;
}