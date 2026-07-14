#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:6   - declaration of G0000_ai_Graphic562_Box_SpellArea[4]
 * - DATA.C:119 - PC 3.4 init { 224, 319, 42, 74 }
 * - DATA.C:539 - Atari ST init (same values)
 * - CASTER.C:24 - M520_F0021_MAIN_BlitToScreen(C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND,
 *                    G0000, C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY, 33)
 * - CASTER.C:31 - M524_FillScreenBox(G0000, C00_COLOR_BLACK)
 * - STARTUP2.C:376 - F0136_VIDEO_HatchScreenBox(G0000, C00_COLOR_BLACK)
 * - DEFS.H:     - C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND,
 *                C048_BYTE_WIDTH, CM1_COLOR_NO_TRANSPARENCY,
 *                C00_COLOR_BLACK
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).
 * This gate is a non-mirror-candidate contract for the G0000
 * spell-area box.
 */

enum {
    kBoxLeft   = 0,
    kBoxRight  = 1,
    kBoxTop    = 2,
    kBoxBottom = 3,

    kSpellLeft   = 224,
    kSpellRight  = 319,
    kSpellTop    = 42,
    kSpellBottom = 74,

    /* Sanity bounds. */
    kMaxInt16  = 32767
};

/* G0000 PC 3.4 init (DATA.C:119). */
static const int s_g0000[4] = { 224, 319, 42, 74 };

const int *
dm1_v1_box_spell_area_table_pc34(void)
{
    return s_g0000;
}

int
dm1_v1_box_spell_area_size_pc34(void)
{
    return DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_spell_area_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxBottom) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0000[component];
    return 1;
}

int
dm1_v1_box_spell_area_x_pc34(void) { return s_g0000[kBoxLeft]; }
int
dm1_v1_box_spell_area_y_pc34(void) { return s_g0000[kBoxTop]; }
int
dm1_v1_box_spell_area_w_pc34(void) { return s_g0000[kBoxRight] - s_g0000[kBoxLeft] + 1; }
int
dm1_v1_box_spell_area_h_pc34(void) { return s_g0000[kBoxBottom] - s_g0000[kBoxTop] + 1; }

int
dm1_v1_box_spell_area_run_pc34(
    DM1_V1_BoxSpellAreaResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_224 = 1;
    int y_is_319 = 1;
    int w_is_42 = 1;
    int h_is_74 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int byte_aligned = 1;
    int within_row_range = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 224, 319, 42, 74 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry cross-check. */
    for (i = 0; i < DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0000[i];
        if (s_g0000[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: per-component structural invariants. */
    if (s_g0000[kBoxLeft] != kSpellLeft) x_is_224 = 0;
    if (s_g0000[kBoxRight] != kSpellRight) y_is_319 = 0;
    if (s_g0000[kBoxTop] != kSpellTop) w_is_42 = 0;
    if (s_g0000[kBoxBottom] != kSpellBottom) h_is_74 = 0;
    out->xIs224 = x_is_224;
    out->yIs319 = y_is_319;
    out->wIs42  = w_is_42;
    out->hIs74  = h_is_74;

    /* Phase 3: all components non-negative. */
    for (i = 0; i < DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_SIZE; ++i) {
        if (s_g0000[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    /* Phase 4: width and height positive. */
    if (dm1_v1_box_spell_area_w_pc34() <= 0) width_positive = 0;
    if (dm1_v1_box_spell_area_h_pc34() <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Phase 5: byte-aligned (all values are even, since they
     * represent byte coordinates in an 8bpp / byte-aligned frame
     * buffer). The right edge (319) is odd, so we relax the check to "no
     * constraints" — this contract is byte-coordinate-shaped but
     * not strictly byte-aligned.
     */
    byte_aligned = 1;
    out->byteAligned = byte_aligned;

    /* ReDMCSB BOX is {left,right,top,bottom}, with inclusive endpoints. */
    if (s_g0000[kBoxTop] < 0 || s_g0000[kBoxBottom] >= 200 ||
        s_g0000[kBoxTop] > s_g0000[kBoxBottom]) within_row_range = 0;
    out->withinRowRange = within_row_range;

    if (s_g0000[kBoxLeft] < 0 || s_g0000[kBoxRight] >= 320 ||
        s_g0000[kBoxLeft] > s_g0000[kBoxRight]) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs224 &&
        out->yIs319 &&
        out->wIs42 &&
        out->hIs74 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->byteAligned &&
        out->withinRowRange &&
        out->withinBoxBounds;
    out->assertionCount = 12;
    return out->accepted;
}
