#include "firestaff/dm1/v1/box_entrance_dungeon_view_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:12   - declaration of G0006_ai_Graphic562_Box_Entrance_DungeonView[4]
 * - DATA.C:131  - PC 3.4 init { 0, 223, 3, 138 }
 * - DATA.C:551  - Atari ST init (same values)
 * - ENTRANCE.C:178/181/184/187 - F0132_VIDEO_Blit entrance
 *   dungeon-view backdrop
 * - DEFS.H:      - C112_BYTE_WIDTH_VIEWPORT, C128_BYTE_WIDTH,
 *                   CM1_COLOR_NO_TRANSPARENCY
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821/830/831/832 (Graphics.dat init-table gates batches 1+2+3+4+
 * 5+6+7+8+9+10+11). This gate is a non-mirror-candidate contract
 * for the G0006 entrance-dungeon-view box.
 */

enum {
    kBoxX      = 0,
    kBoxY      = 1,
    kBoxW      = 2,
    kBoxH      = 3,

    kEntrX     = 0,
    kEntrY     = 223,
    kEntrW     = 3,
    kEntrH     = 138
};

static const int s_g0006[4] = { 0, 223, 3, 138 };

const int *
dm1_v1_box_entrance_dungeon_view_table_pc34(void)
{
    return s_g0006;
}

int
dm1_v1_box_entrance_dungeon_view_size_pc34(void)
{
    return DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE;
}

int
dm1_v1_box_entrance_dungeon_view_get_pc34(int component, int *out_value)
{
    if (!out_value) {
        return 0;
    }
    if (component < 0 || component > kBoxH) {
        *out_value = -1;
        return 0;
    }
    *out_value = s_g0006[component];
    return 1;
}

int
dm1_v1_box_entrance_dungeon_view_x_pc34(void) { return s_g0006[kBoxX]; }
int
dm1_v1_box_entrance_dungeon_view_y_pc34(void) { return s_g0006[kBoxY]; }
int
dm1_v1_box_entrance_dungeon_view_w_pc34(void) { return s_g0006[kBoxW]; }
int
dm1_v1_box_entrance_dungeon_view_h_pc34(void) { return s_g0006[kBoxH]; }

int
dm1_v1_box_entrance_dungeon_view_run_pc34(
    DM1_V1_BoxEntranceDungeonViewResultPc34 *out)
{
    int table_matches_declaration = 1;
    int x_is_0 = 1;
    int y_is_223 = 1;
    int w_is_3 = 1;
    int h_is_138 = 1;
    int all_components_non_negative = 1;
    int width_positive = 1;
    int height_positive = 1;
    int within_viewport_width = 1;
    int within_box_bounds = 1;
    static const int kExpected[4] = { 0, 223, 3, 138 };
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE; ++i) {
        out->tableEntries[i] = s_g0006[i];
        if (s_g0006[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE;
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_g0006[kBoxX] != kEntrX) x_is_0 = 0;
    if (s_g0006[kBoxY] != kEntrY) y_is_223 = 0;
    if (s_g0006[kBoxW] != kEntrW) w_is_3 = 0;
    if (s_g0006[kBoxH] != kEntrH) h_is_138 = 0;
    out->xIs0   = x_is_0;
    out->yIs223 = y_is_223;
    out->wIs3   = w_is_3;
    out->hIs138 = h_is_138;

    for (i = 0; i < DM1_V1_BOX_ENTRANCE_DUNGEON_VIEW_PC34_COMPAT_SIZE; ++i) {
        if (s_g0006[i] < 0) {
            all_components_non_negative = 0;
        }
    }
    out->allComponentsNonNegative = all_components_non_negative;

    if (s_g0006[kBoxW] <= 0) width_positive = 0;
    if (s_g0006[kBoxH] <= 0) height_positive = 0;
    out->widthPositive  = width_positive;
    out->heightPositive = height_positive;

    /* Y=223 is just within 320-line render buffer. */
    if (s_g0006[kBoxY] < 0 || s_g0006[kBoxY] > 320) within_viewport_width = 0;
    out->withinViewportWidth = within_viewport_width;

    /* X=0, W=3 → end=3 (very small box, well within bounds). */
    if (s_g0006[kBoxX] + s_g0006[kBoxW] > 320) within_box_bounds = 0;
    out->withinBoxBounds = within_box_bounds;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->xIs0 &&
        out->yIs223 &&
        out->wIs3 &&
        out->hIs138 &&
        out->allComponentsNonNegative &&
        out->widthPositive &&
        out->heightPositive &&
        out->withinViewportWidth &&
        out->withinBoxBounds;
    out->assertionCount = 11;
    return out->accepted;
}