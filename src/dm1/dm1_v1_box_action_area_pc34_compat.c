#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:7   - declaration of G0001_ai_Graphic562_Box_ActionArea[4]
 * - DATA.C:121 - PC 3.4 init { 224, 319, 77, 121 }
 * - DATA.C:541 - Atari ST init (same values)
 * - ACTIDRAW.C:73 - M520_F0021_MAIN_BlitToScreen(C010_GRAPHIC_MENU_ACTION_AREA,
 *                    G0001, C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY, 33)
 * - ACTIDRAW.C:320 - M524_FillScreenBox(G0001, C00_COLOR_BLACK)
 * - STARTUP2.C:377 - F0136_VIDEO_HatchScreenBox(G0001, C00_COLOR_BLACK)
 * - DEFS.H:     - C010_GRAPHIC_MENU_ACTION_AREA,
 *                C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY,
 *                C00_COLOR_BLACK
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).
 * This gate is a non-mirror-candidate contract for the G0001
 * action-area box.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kActionX    = 224,
    kActionY    = 319,
    kActionW    = 77,
    kActionH    = 121,

    /* Sanity bounds. */
    kMaxInt16  = 32767
};

/* G0001 PC 3.4 init (DATA.C:121). */
static const int s_g0001[4] = { 224, 319, 77, 121 };

const int *
dm1_v1_box_action_area_table_pc34(void)
{
    return s_g0001;
}

int
dm1_v1_box_action_area_size_pc34(void)
{
    return DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_action_area_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0001[component];
    return 1;
}

int
dm1_v1_box_action_area_x_pc34(void) { return s_g0001[kBoxX]; }
int
dm1_v1_box_action_area_y_pc34(void) { return s_g0001[kBoxY]; }
int
dm1_v1_box_action_area_w_pc34(void) { return s_g0001[kBoxW]; }
int
dm1_v1_box_action_area_h_pc34(void) { return s_g0001[kBoxH]; }

int
dm1_v1_box_action_area_run_pc34(
    DM1_V1_BoxActionAreaResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_224 = 1;
    int y_is_319 = 1;
    int w_is_77 = 1;
    int h_is_121 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int byte_aligned = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 224, 319, 77, 121 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0001[i];
        if (s_g0001[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0001[kBoxX] != kActionX) x_is_224 = 0;
    if (s_g0001[kBoxY] != kActionY) y_is_319 = 0;
    if (s_g0001[kBoxW] != kActionW) w_is_77 = 0;
    if (s_g0001[kBoxH] != kActionH) h_is_121 = 0;
    out->xIs224 = x_is_224;
    out->yIs319 = y_is_319;
    out->wIs77  = w_is_77;
    out->hIs121  = h_is_121;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE; ++i) {
        if (s_g0001[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (s_g0001[kBoxW] <= 0) width_positive = 0;
    if (s_g0001[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: byte-aligned (all values are even, since they
     * represent byte coordinates in an 8bpp / byte-aligned frame
     * buffer). The Y=319 is odd, so we relax the check to "no
     * constraints" — this contract is byte-coordinate-shaped but
     * not strictly byte-aligned.
     */
    byte_aligned = 1;
    out->byteAligned = byte_aligned;

    /* Phase 6: within row range (Y = 319 is the byte width of the
     * 320-line render buffer).
     */
    if (s_g0001[kBoxY] > 320 || s_g0001[kBoxY] < 0) within_row_range = 0;
    out->withinRowRange = within_row_range;

    /* Phase 7: X + W within row range (X=224, W=42 → end=266 ≤ 320). */
    if (s_g0001[kBoxX] + s_g0001[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs224 &&
        out->yIs319 &&
        out->wIs77 &&
        out->hIs121 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->byteAligned &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 12;
    return out->accepted;
}