#include "csb/csb_v1_viewport_f0115_wall_text_ornament_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D1C = 3,      /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C. */
    CSB_VIEW_DEPTH_D1 = 1,        /* ReDMCSB DUNVIEW.C:372 G2027[3]. */
    CSB_VIEW_LANE_CENTER = 0,     /* ReDMCSB DUNVIEW.C:371 G2026[3]. */
    CSB_ELEMENT_WALL = 0,
    CSB_M552_FRONT_WALL_ORNAMENT_ORDINAL = 5,
    CSB_M587_VIEW_WALL_D1C_FRONT = 14, /* ReDMCSB DEFS.H:2710. */
    CSB_WALL_TEXT_ORDINAL = 1,
    CSB_WALL_TEXT_INDEX = 0,
    CSB_WALL_TEXT_COORDINATE_SET = 0,
    CSB_C5_HEIGHT_INDEX = 5,      /* ReDMCSB DEFS.H:2527 C5_HEIGHT. */
    CSB_C10_COLOR_FLESH = 10,     /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_CELL_ORDER_ALCOVE = 0x0000,
    CSB_TEXT_X = 88,
    CSB_TEXT_Y = 48,
    CSB_TEXT_W = 48,
    CSB_TEXT_H = 24,
    CSB_WALL_COLOR = 3,
    CSB_TEXT_COLOR = 15,
    CSB_VIEWPORT_SENTINEL = 0xee
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 branch-A gate; synthetic framebuffer "
    "fixture only, no original DOS pixel parity claim and no CSB game-data "
    "load. ReDMCSB DUNVIEW.C:7825-7843 F0124_DUNGEONVIEW_DrawSquareD1C "
    "locks the D1C wall route: the wall bitmap is drawn, "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF is called with "
    "M552_FRONT_WALL_ORNAMENT_ORDINAL and M587_VIEW_WALL_D1C_FRONT at 7842, "
    "and a true alcove result calls F0115 with M606_VIEW_SQUARE_D1C and "
    "C0x0000_CELL_ORDER_ALCOVE at 7843. DUNVIEW.C:3502-3938 F0107 locks the "
    "wall-ornament ordinal path: 3571-3578 rejects ordinal zero, decrements "
    "to an ornament index and selects the coordinate set; 3589 records the "
    "alcove flag; 3590-3717 draws inscription text for D1C front wall text; "
    "3907-3910 blits the wall ornament with C10 transparency; 3923-3928 "
    "updates the D1C clickable/portrait overlay path and 3932-3938 returns "
    "the alcove flag. DUNVIEW.C:4547-4581 F0115 defines the thing-pass order "
    "and states that first nibble zero is the wall-alcove object path. "
    "DUNVIEW.C:8318-8486 F0128 locks the frame setup and early far-square "
    "F0115 sequence before the D1 routes. DEFS.H:2088 anchors C10, "
    "DEFS.H:2527 anchors C5_HEIGHT, DEFS.H:4139-4153 anchors the nearby "
    "zone family, and DEFS.H:5576 declares G0208 door-button coordinate-set "
    "storage used by adjacent wall control coordinates. CSB-lineage "
    "Viewport.cpp:1192-1209 keeps the F0 open room-object route, "
    "1903-1915 keeps F1 door-facing object order, and 1930-1944 keeps the "
    "F0 door-facing return/open route cross-check.";

static const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec s_spec = {
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34,
    CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34,
    CSB_V1_F0115_WALL_TEXT_ORNAMENT_VIEWPORT_WIDTH_PC34,
    CSB_V1_F0115_WALL_TEXT_ORNAMENT_VIEWPORT_HEIGHT_PC34,
    CSB_VIEW_SQUARE_D1C,
    CSB_VIEW_DEPTH_D1,
    CSB_VIEW_LANE_CENTER,
    CSB_ELEMENT_WALL,
    CSB_M552_FRONT_WALL_ORNAMENT_ORDINAL,
    CSB_M587_VIEW_WALL_D1C_FRONT,
    CSB_WALL_TEXT_ORDINAL,
    CSB_WALL_TEXT_INDEX,
    CSB_WALL_TEXT_COORDINATE_SET,
    CSB_C5_HEIGHT_INDEX,
    CSB_C10_COLOR_FLESH,
    CSB_PRESENT,
    CSB_CELL_ORDER_ALCOVE,
    CSB_PRESENT,
    CSB_TEXT_X,
    CSB_TEXT_Y,
    CSB_TEXT_W,
    CSB_TEXT_H,
    "ReDMCSB DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581",
    "ReDMCSB DUNVIEW.C:F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF:3502-3938",
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7825-7843",
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8318-8486",
    "ReDMCSB DEFS.H:C10:2088; C5_HEIGHT:2527; M606/M587:2599/2710; zones:4139-4153; G0208:5576",
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915,1930-1944",
    s_source_evidence
};

static size_t offset_for(int x, int y)
{
    return (size_t)y *
           CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 +
           (size_t)x;
}

static int text_mask_pixel(int x, int y)
{
    const int local_x = x - CSB_TEXT_X;
    const int local_y = y - CSB_TEXT_Y;

    if (local_x < 0 || local_y < 0 ||
        local_x >= CSB_TEXT_W || local_y >= CSB_TEXT_H) {
        return CSB_ABSENT;
    }

    if ((local_y % 8) == 0 || (local_y % 8) == 7) return CSB_PRESENT;
    if ((local_x % 12) == 1 || (local_x % 12) == 10) return CSB_PRESENT;
    if ((local_x + local_y) % 11 == 0) return CSB_PRESENT;
    return CSB_ABSENT;
}

const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *
csb_v1_viewport_f0115_wall_text_ornament_pc34_spec(void)
{
    return &s_spec;
}

const char *
csb_v1_viewport_f0115_wall_text_ornament_pc34_source_evidence(void)
{
    return s_source_evidence;
}

int csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
    const CSB_V1_ViewportF0115WallTextOrnamentPc34Spec *spec,
    int wall_ornament_ordinal,
    int view_wall_index)
{
    if (!spec) return CSB_ABSENT;
    if (wall_ornament_ordinal != spec->wall_text_ornament_ordinal) {
        return CSB_ABSENT;
    }
    if (view_wall_index != spec->d1c_front_wall_view_index) {
        return CSB_ABSENT;
    }
    return spec->f0107_reports_alcove &&
           spec->f0115_cell_order_alcove == CSB_CELL_ORDER_ALCOVE;
}

uint32_t csb_v1_viewport_f0115_wall_text_ornament_hash_pc34(
    const uint8_t *framebuffer,
    size_t framebuffer_size)
{
    uint32_t hash = 2166136261u;

    if (!framebuffer) return 0u;
    for (size_t i = 0; i < framebuffer_size; ++i) {
        hash ^= framebuffer[i];
        hash *= 16777619u;
    }
    return hash;
}

int csb_v1_viewport_f0115_wall_text_ornament_pixel_pc34(
    const uint8_t *framebuffer,
    size_t framebuffer_size,
    int x,
    int y)
{
    const size_t needed =
        (size_t)CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 *
        (size_t)CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34;

    if (!framebuffer || framebuffer_size < needed) return -1;
    if (x < 0 ||
        x >= CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34) {
        return -1;
    }
    if (y < 0 ||
        y >= CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34) {
        return -1;
    }
    return framebuffer[offset_for(x, y)];
}

int csb_v1_viewport_f0115_wall_text_ornament_render_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size,
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace *out_trace)
{
    const size_t needed =
        (size_t)CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_WIDTH_PC34 *
        (size_t)CSB_V1_F0115_WALL_TEXT_ORNAMENT_FRAMEBUFFER_HEIGHT_PC34;
    CSB_V1_ViewportF0115WallTextOrnamentPc34Trace trace = {
        0, 0, 0, 0, 0, 0, 1, 2, -1, -1, 0u, s_source_evidence
    };

    if (!framebuffer || !out_trace || framebuffer_size < needed) return -1;

    trace.route_enabled =
        csb_v1_viewport_f0115_wall_text_ornament_route_enabled_pc34(
            &s_spec,
            s_spec.wall_text_ornament_ordinal,
            s_spec.d1c_front_wall_view_index);
    if (!trace.route_enabled) return 1;

    for (int y = 0; y < s_spec.viewport_height; ++y) {
        for (int x = 0; x < s_spec.viewport_width; ++x) {
            framebuffer[offset_for(x, y)] = (uint8_t)CSB_WALL_COLOR;
            ++trace.wall_pixels;
        }
    }

    /* ReDMCSB: DUNVIEW.C F0107 lines 3590-3717 draw D1C front text,
     * using C10 transparent font/ornament blits; F0115 line 4562 then
     * treats cell-order zero as the matching wall-alcove thing route. */
    for (int y = s_spec.text_box_y;
         y < s_spec.text_box_y + s_spec.text_box_height; ++y) {
        for (int x = s_spec.text_box_x;
             x < s_spec.text_box_x + s_spec.text_box_width; ++x) {
            if (text_mask_pixel(x, y)) {
                framebuffer[offset_for(x, y)] = (uint8_t)CSB_TEXT_COLOR;
                if (trace.first_text_x < 0) {
                    trace.first_text_x = x;
                    trace.first_text_y = y;
                }
                ++trace.text_pixels_non_zero;
            } else if (framebuffer[offset_for(x, y)] ==
                       (uint8_t)CSB_WALL_COLOR) {
                ++trace.transparent_pixels_preserved;
            }
        }
    }

    trace.outside_viewport_preserved =
        framebuffer[offset_for(s_spec.viewport_width, 0)] ==
        (uint8_t)CSB_VIEWPORT_SENTINEL;
    trace.framebuffer_hash =
        csb_v1_viewport_f0115_wall_text_ornament_hash_pc34(
            framebuffer, framebuffer_size);
    trace.ok = trace.wall_pixels ==
                   (s_spec.viewport_width * s_spec.viewport_height) &&
               trace.text_pixels_non_zero > 0 &&
               trace.transparent_pixels_preserved > 0 &&
               trace.outside_viewport_preserved &&
               trace.f0107_order < trace.f0115_order;

    *out_trace = trace;
    return trace.ok ? 0 : 1;
}
