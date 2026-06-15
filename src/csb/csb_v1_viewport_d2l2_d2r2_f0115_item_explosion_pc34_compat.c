#include "csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_D2L2_VIEW_SQUARE = 9,             /* ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2 */
    CSB_D2R2_VIEW_SQUARE = 10,            /* ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2 */
    CSB_D2_VIEW_DEPTH = 2,                /* ReDMCSB DUNVIEW.C:372 G2027[9/10] */
    CSB_D2L2_VIEW_LATERAL = -2,           /* ReDMCSB DUNVIEW.C:8503 F0128 */
    CSB_D2R2_VIEW_LATERAL = 2,            /* ReDMCSB DUNVIEW.C:8507 F0128 */
    CSB_D2L2_VIEW_LANE = 254,             /* ReDMCSB DUNVIEW.C:371 G2026[9] == -2 */
    CSB_D2R2_VIEW_LANE = 2,               /* ReDMCSB DUNVIEW.C:371 G2026[10] */
    CSB_MISSING_ROW = -1,                 /* ReDMCSB DUNVIEW.C:373/376 G2028/G2034[9/10] */
    CSB_D2L2_FIELD_ASPECT = 5,            /* ReDMCSB DUNVIEW.C:377 G2035[9] */
    CSB_D2R2_FIELD_ASPECT = 6,            /* ReDMCSB DUNVIEW.C:377 G2035[10] */
    CSB_D2L2_WALL_ZONE = 707,             /* ReDMCSB DEFS.H:4047 C707_ZONE_WALL_D2L2 */
    CSB_D2R2_WALL_ZONE = 708,             /* ReDMCSB DEFS.H:4048 C708_ZONE_WALL_D2R2 */
    CSB_OBJECT_ZONE_BASE = 2500,          /* ReDMCSB DEFS.H:4228 C2500_ZONE_ */
    CSB_OBJECT_ZONE_STRIDE = 4,           /* ReDMCSB DUNVIEW.C:5075 row*4+ViewCell */
    CSB_OBJECT_SHIFT_MASK = 0x8000,       /* ReDMCSB DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES */
    CSB_EXPLOSION_REBIRTH1_ZONE = 3000,   /* ReDMCSB DEFS.H:4232 C3000_ZONE_ */
    CSB_EXPLOSION_REBIRTH2_ZONE = 3007,   /* ReDMCSB DEFS.H:4233 C3007_ZONE_ */
    CSB_EXPLOSION_CENTERED_ZONE = 3014,   /* ReDMCSB DEFS.H:4234 C3014_ZONE_ */
    CSB_EXPLOSION_SIDE_ZONE = 3031,       /* ReDMCSB DEFS.H:4235 C3031_ZONE_ */
    CSB_EXPLOSION_SIDE_STRIDE = 2,        /* ReDMCSB DUNVIEW.C:6121-6122 row*2+ViewCell */
    CSB_TRANSPARENT_COLOR = 10,           /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH */
    CSB_LINEAGE_RF2L2 = 3,                /* CSB Viewport.cpp:334 RF2L2 */
    CSB_LINEAGE_RF2R2 = 11,               /* CSB Viewport.cpp:339 RF2R2 */
    CSB_LINEAGE_STD_DRAW_ROOM_OBJECTS = 60006, /* CSB Viewport.cpp:379 */
    CSB_LINEAGE_F2L1_CONTENTS = 60122,    /* CSB Viewport.cpp:508 */
    CSB_LINEAGE_F2R1_CONTENTS = 60124,    /* CSB Viewport.cpp:510 */
    CSB_LINEAGE_DRAW_ORDER_218 = 60279,   /* CSB Viewport.cpp:681 */
    CSB_LINEAGE_DRAW_ORDER_128 = 60283    /* CSB Viewport.cpp:685 */
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:371-377 maps C09/C10 D2L2/D2R2 to depth 2, lanes "
    "-2/+2, G2028=-1, G2034=-1, and field aspects 5/6. F0115 item rows: "
    "4806-4811 load G2028, 4923 gates weapon..junk on L2476>=0 and cell "
    "match, 5030-5039 select PC34/I34 object scale/shift, 5071-5079 bind "
    "C2500_ZONE_ | MASK0x8000 + row*4 + ViewCell and pile shifts, 5082-5089 "
    "advance/wrap the pile shift, and 5109 blits through F0791 with C10. "
    "F0115 explosion rows: 5915-5933 restart from the thing list, 5920-5924 "
    "load G2034/G2035, 5948 rejects rebirth when row <0 or cell mismatches, "
    "5998-5999/6094-6096/6106-6107/6121-6122 bind C3000/C3007/C3014/C3031, "
    "6192-6193 blit through F0791 with C10, and 6202-6219 defer fluxcage to "
    "F0113 at C702 + field aspect. DUNVIEW.C:6837-6896 F0678/F0679 expose "
    "no D2L2/D2R2 F0115/F0111 door-front route; wall cases return and "
    "teleporters call F0113 with C707/C708. DUNVIEW.C:4218-4334 F0111 is "
    "only a door panel route and skips open doors. DEFS.H:2088/2605-2606/"
    "3517/4047-4048/4228/4232-4235/4250 and COORD.C:1058-1123/1129-1193 "
    "anchor the zones. CSB Viewport.cpp:334/339 RF2L2/RF2R2, 379 "
    "StdDrawRoomObjects, 508/510 F2 contents opcodes, and 681/685 "
    "DrawOrder218/DrawOrder128 provide the CSB-lineage binding.";

static const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec s_specs[] = {
    {
        CSB_D2L2_VIEW_SQUARE,
        CSB_D2L2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2L2_VIEW_LATERAL,
        CSB_D2L2_VIEW_LANE,
        CSB_MISSING_ROW,
        CSB_MISSING_ROW,
        CSB_D2L2_FIELD_ASPECT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_D2L2_WALL_ZONE,
        CSB_ROUTE_PRESENT,
        CSB_OBJECT_ZONE_BASE,
        CSB_OBJECT_ZONE_STRIDE,
        CSB_OBJECT_SHIFT_MASK,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_EXPLOSION_REBIRTH1_ZONE,
        CSB_EXPLOSION_REBIRTH2_ZONE,
        CSB_EXPLOSION_CENTERED_ZONE,
        CSB_EXPLOSION_SIDE_ZONE,
        CSB_EXPLOSION_SIDE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_D2L2_WALL_ZONE,
        CSB_TRANSPARENT_COLOR,
        CSB_LINEAGE_RF2L2,
        CSB_LINEAGE_F2L1_CONTENTS,
        CSB_LINEAGE_STD_DRAW_ROOM_OBJECTS,
        CSB_LINEAGE_DRAW_ORDER_218,
        CSB_LINEAGE_DRAW_ORDER_128,
        "DUNVIEW.C F0678_DrawD2L2 / F0115 item-explosion metadata",
        s_source_evidence
    },
    {
        CSB_D2R2_VIEW_SQUARE,
        CSB_D2R2_VIEW_SQUARE,
        CSB_D2_VIEW_DEPTH,
        CSB_D2R2_VIEW_LATERAL,
        CSB_D2R2_VIEW_LANE,
        CSB_MISSING_ROW,
        CSB_MISSING_ROW,
        CSB_D2R2_FIELD_ASPECT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_D2R2_WALL_ZONE,
        CSB_ROUTE_PRESENT,
        CSB_OBJECT_ZONE_BASE,
        CSB_OBJECT_ZONE_STRIDE,
        CSB_OBJECT_SHIFT_MASK,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_EXPLOSION_REBIRTH1_ZONE,
        CSB_EXPLOSION_REBIRTH2_ZONE,
        CSB_EXPLOSION_CENTERED_ZONE,
        CSB_EXPLOSION_SIDE_ZONE,
        CSB_EXPLOSION_SIDE_STRIDE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_D2R2_WALL_ZONE,
        CSB_TRANSPARENT_COLOR,
        CSB_LINEAGE_RF2R2,
        CSB_LINEAGE_F2R1_CONTENTS,
        CSB_LINEAGE_STD_DRAW_ROOM_OBJECTS,
        CSB_LINEAGE_DRAW_ORDER_218,
        CSB_LINEAGE_DRAW_ORDER_128,
        "DUNVIEW.C F0679_DrawD2R2 / F0115 item-explosion metadata",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *
csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *
csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_count_pc34(); ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return NULL;
}

int csb_v1_viewport_d2l2_d2r2_f0115_item_layout_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->object_g2028_row < 0) return -1;
    return spec->object_zone_base +
           (spec->object_g2028_row * spec->object_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell)
{
    const int zone = csb_v1_viewport_d2l2_d2r2_f0115_item_layout_zone_pc34(spec, view_cell);
    if (zone < 0) return -1;
    return zone | spec->object_shift_mask;
}

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step1_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_rebirth_step1_zone_base + spec->explosion_g2034_row;
}

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step2_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_rebirth_step2_zone_base + spec->explosion_g2034_row;
}

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_centered_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec)
{
    if (!spec || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_centered_zone_base + spec->explosion_g2034_row;
}

int csb_v1_viewport_d2l2_d2r2_f0115_explosion_side_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    unsigned char view_cell)
{
    if (!spec || view_cell > 1 || spec->explosion_g2034_row < 0) return -1;
    return spec->explosion_side_zone_base +
           (spec->explosion_g2034_row * spec->explosion_side_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_d2l2_d2r2_f0115_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5109 and 6192-6193 send item and
     * explosion bitmaps through F0791 with C10_COLOR_FLESH transparency. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

const char *csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_source_evidence_pc34(void)
{
    return s_source_evidence;
}
