#include "csb_v1_viewport_d3l2_f0115_projectile_pc34_compat.h"

enum {
    CSB_D3L2_VIEW_SQUARE = 14,          /* ReDMCSB DEFS.H:2610 C14_VIEW_SQUARE_D3L2 */
    CSB_D3R2_VIEW_SQUARE = 15,          /* ReDMCSB DEFS.H:2611 C15_VIEW_SQUARE_D3R2 */
    CSB_D3_VIEW_DEPTH = 3,              /* ReDMCSB DUNVIEW.C:372 G2027[14] */
    CSB_D3L2_VIEW_LANE = 254,           /* ReDMCSB DUNVIEW.C:371 G2026[14] == -2 */
    CSB_D3L2_G2028_ROW = 3,             /* ReDMCSB DUNVIEW.C:373 G2028[14] */
    CSB_D3R2_G2028_ROW = 4,             /* ReDMCSB DUNVIEW.C:373 G2028[15] */
    CSB_PROJECTILE_ZONE_BASE = 2900,    /* ReDMCSB DEFS.H:4230 C2900_ZONE_ */
    CSB_PROJECTILE_ZONE_STRIDE = 4,     /* ReDMCSB DUNVIEW.C:5683 row*4+ViewCell */
    CSB_PROJECTILE_THING_TYPE = 14,     /* ReDMCSB DEFS.H C14_THING_TYPE_PROJECTILE */
    CSB_PROJECTILE_DERIVED_NONE = -1,   /* ReDMCSB DUNVIEW.C:5885 CM1_DERIVED_BITMAP_NONE */
    CSB_KINETIC_MINIMUM = 96,           /* ReDMCSB DUNVIEW.C:5720 max(96, energy + 1) */
    CSB_KINETIC_FLOOR = 2,              /* ReDMCSB DUNVIEW.C:5720 max(2, scaled) */
    CSB_TRANSPARENT_COLOR = 10          /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity is claimed. "
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 orders object, creature, projectile, "
    "then explosion passes. DUNVIEW.C:371-373 maps C14_VIEW_SQUARE_D3L2 to "
    "depth 3, lane -2, and G2028 row 3; C15_VIEW_SQUARE_D3R2 maps to row 4 "
    "and is intentionally excluded from this D3L2 gate. DUNVIEW.C:5668-5683 "
    "loads G2028, suppresses depth-3 front cells, restarts the thing list, "
    "requires C14_THING_TYPE_PROJECTILE and M011_CELL match, then computes "
    "C2900_ZONE_ + row*4 + ViewCell. DUNVIEW.C:5710-5722 derives scale index "
    "as (ViewDepth << 1) - (ViewCell >> 1) and applies kinetic-energy scaling "
    "with max(96, energy + 1) and max(2, scaled). DUNVIEW.C:5881-5882 blits "
    "the scaled projectile through F0791 with the dynamic flip flag and "
    "C10_COLOR_FLESH transparency. DEFS.H:2610-2611 defines C14/C15 "
    "D3L2/D3R2 view squares; DEFS.H:4230 defines C2900_ZONE_; DEFS.H:2088 "
    "defines C10_COLOR_FLESH. Non-overlap: D3L2 wall F0121/F0104 and D3L2 "
    "teleporter F0113 are not covered by this projectile slice.";

static const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 s_route = {
    CSB_D3L2_VIEW_SQUARE,
    CSB_D3_VIEW_DEPTH,
    CSB_D3L2_VIEW_LANE,
    CSB_D3L2_G2028_ROW,
    CSB_D3R2_G2028_ROW,
    CSB_PROJECTILE_ZONE_BASE,
    CSB_PROJECTILE_ZONE_STRIDE,
    CSB_PROJECTILE_THING_TYPE,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    CSB_KINETIC_MINIMUM,
    CSB_KINETIC_FLOOR,
    CSB_PROJECTILE_DERIVED_NONE,
    1,
    1,
    CSB_TRANSPARENT_COLOR,
    0,
    0,
    1,
    "DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions",
    s_source_evidence
};

const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *
csb_v1_viewport_d3l2_f0115_projectile_route_spec_pc34(void)
{
    return &s_route;
}

int csb_v1_viewport_d3l2_f0115_projectile_accepts_thing_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    int thing_type,
    unsigned char thing_cell,
    unsigned char view_cell)
{
    if (!spec || view_cell > 3) return 0;
    if (spec->requires_projectile_type_c14 && thing_type != spec->projectile_thing_type) {
        return 0;
    }
    if (spec->requires_cell_match && thing_cell != view_cell) return 0;
    return csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, view_cell) >= 0;
}

int csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->projectile_g2028_row < 0) return -1;
    if (spec->view_depth == 3 && spec->suppresses_depth3_front_cells && view_cell <= 1) {
        return -1;
    }
    if (spec->view_depth == 0 && spec->suppresses_depth0_back_cells && view_cell >= 2) {
        return -1;
    }
    return spec->projectile_zone_base +
           (spec->projectile_g2028_row * spec->projectile_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d3l2_f0115_projectile_scale_index_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    unsigned char view_cell)
{
    if (csb_v1_viewport_d3l2_f0115_projectile_zone_pc34(spec, view_cell) < 0) {
        return -1;
    }
    return (spec->view_depth << spec->projectile_scale_index_depth_shift) -
           (view_cell >> spec->projectile_scale_index_cell_shift);
}

int csb_v1_viewport_d3l2_f0115_projectile_apply_kinetic_scale_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    int base_scale,
    int kinetic_energy,
    int scales_with_kinetic_energy)
{
    int kinetic;
    int scaled;

    if (!spec || base_scale < 0 || kinetic_energy < 0) return -1;
    if (!scales_with_kinetic_energy) return base_scale;
    kinetic = kinetic_energy + 1;
    if (kinetic < spec->projectile_kinetic_minimum) kinetic = spec->projectile_kinetic_minimum;
    scaled = (base_scale * kinetic) >> 8;
    return scaled < spec->projectile_kinetic_floor ? spec->projectile_kinetic_floor : scaled;
}

int csb_v1_viewport_d3l2_f0115_projectile_apply_c10_blit_pc34(
    const CSB_V1_ViewportD3L2F0115ProjectileRouteSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height,
    int flip_horizontal)
{
    int copied = 0;

    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5881-5882 routes the D3L2 projectile
     * bitmap through F0791 using L2474_i_ZoneIndex, dynamic flip flags, and
     * C10_COLOR_FLESH transparency. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int sx = flip_horizontal ? width - 1 - x : x;
            const uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

const char *csb_v1_viewport_d3l2_f0115_projectile_source_evidence_pc34(void)
{
    return s_source_evidence;
}
