#include "firestaff/dm1/v1/viewport/d2l2_d2r2_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_C09_VIEW_SQUARE_D2L2 = 9,
    DM1_C10_VIEW_SQUARE_D2R2 = 10,
    DM1_M604_VIEW_SQUARE_D2L = 7,
    DM1_M605_VIEW_SQUARE_D2R = 8,
    DM1_C707_ZONE_WALL_D2L2 = 707,
    DM1_C708_ZONE_WALL_D2R2 = 708,
    DM1_C710_ZONE_WALL_D2L = 710,
    DM1_C711_ZONE_WALL_D2R = 711,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_D2_WALL_ORNAMENT_COORDINATE_SET = 2,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_M580_VIEW_WALL_D2L_RIGHT = 7,
    DM1_M581_VIEW_WALL_D2R_LEFT = 8
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0678/F0679:6837-6896 draw the C09/C10 D2L2/D2R2 guard
 *   wall carriers C707/C708 for later media and never call F0107.
 * - DUNVIEW.C F0119:6900-7049 and F0120:7051-7224 are the D2L/D2R
 *   bodies carrying the D2L2/D2R2 side-wall ornament calls: F0119 line
 *   6968 uses M551_RIGHT_WALL_ORNAMENT_ORDINAL with
 *   M580_VIEW_WALL_D2L_RIGHT; F0120 line 7119 uses
 *   M553_LEFT_WALL_ORNAMENT_ORDINAL with M581_VIEW_WALL_D2R_LEFT.
 * - DUNVIEW.C F0107:3502-3938 owns C1004 + CoordinateSet * 15 +
 *   ViewWall zone math, C10 transparent blit, and the alcove boolean.
 * - DUNVIEW.C F0108:3940-4011 is a separate floor/ceiling baseline.
 * - DUNVIEW.C F0128:8503-8521 dispatches D2L2, then D2R2, then D2L,
 *   D2R, and D2C.
 * - DUNGEON.C F0163:1769-1838, F0164:1840-1905, and F0172:2466-2523
 *   anchor thing-list and square-aspect feeds.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0678/F0679:6837-6896 expose the "
    "local C09/C10 D2L2/D2R2 guard wall carriers. The requested C711/C712 "
    "labels do not match this local source: DEFS.H:4047-4048 defines "
    "C707_ZONE_WALL_D2L2/C708_ZONE_WALL_D2R2, while C711 is D2R and C712 "
    "is not D2R2. DUNVIEW.C F0119:6900-7049 and F0120:7051-7224 are the "
    "D2L/D2R carrier bodies for the D2L2/D2R2 side wall ornaments. "
    "F0119 line 6968 calls F0107 with M551_RIGHT_WALL_ORNAMENT_ORDINAL "
    "and M580_VIEW_WALL_D2L_RIGHT; F0120 line 7119 calls F0107 with "
    "M553_LEFT_WALL_ORNAMENT_ORDINAL and M581_VIEW_WALL_D2R_LEFT. "
    "DUNVIEW.C F0107:3502-3938 decrements non-zero wall-ornament ordinals "
    "at 3571-3575, computes C1004_ZONE_WALL_ORNAMENT + CoordinateSet * "
    "C15 + ViewWall at 3586-3587, classifies alcoves at 3589, draws with "
    "C10_COLOR_FLESH at 3922, and returns the alcove flag at 3933. "
    "DUNVIEW.C F0108:3940-4011 is the separate floor/ceiling baseline. "
    "DUNVIEW.C F0128:8503-8521 dispatches D2L2 at 8503-8504, D2R2 at "
    "8507-8508, D2L at 8512-8513, D2R at 8516-8517, and D2C at "
    "8520-8521. DUNGEON.C F0163:1769-1838, F0164:1840-1905, and "
    "F0172:2466-2523 anchor thing-list and square-aspect feeds. "
    "DEFS.H:2088 anchors C10_COLOR_FLESH; DEFS.H:2551/2553 anchor "
    "M551/M553; DEFS.H:2603-2606 anchors M604/M605/C09/C10; "
    "DEFS.H:2703-2704 anchors M580/M581; DEFS.H:4222 anchors C1004.";

static const char s_disjointness_note[] =
    "D2L2/D2R2 F0107 wall-ornament source-lock contract only. It is "
    "distinct from D0L/D0R, D1C, D1L/D1R, D2C, D2L/D2R, D3L/D3R, D3C, "
    "and CSB-lineage F0107 gates by relative cells (2,-2)/(2,+2), guard "
    "carrier zones C707/C708, side-wall view ordinals M580/M581, and the "
    "synthetic 16x31 probe aspect ratio. It is asset-free, reads no "
    "GRAPHICS.DAT, and makes no original DOS pixel parity claim.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int rects_overlap(const DM1_V1_D2L2D2R2F0107RectPc34 *a,
                         const DM1_V1_D2L2D2R2F0107RectPc34 *b)
{
    if (!a || !b) return 0;
    return a->x < b->x + b->width &&
           a->x + a->width > b->x &&
           a->y < b->y + b->height &&
           a->y + a->height > b->y;
}

int dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

uint8_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int side_index,
    int ornament_index_c0_to_c5)
{
    return side_index >= 0 &&
           side_index < DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34 &&
           ornament_index_c0_to_c5 >= 0 &&
           ornament_index_c0_to_c5 < DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34;
}

static void fill_lanes(DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    m->lanes[0] = (DM1_V1_D2L2D2R2F0107LanePc34){
        DM1_V1_D2L2_D2R2_F0107_SIDE_D2L2_PC34,
        "D2L2",
        DM1_C09_VIEW_SQUARE_D2L2,
        DM1_M604_VIEW_SQUARE_D2L,
        2,
        -2,
        DM1_C707_ZONE_WALL_D2L2,
        DM1_C710_ZONE_WALL_D2L,
        6837,
        6865,
        6900,
        7049,
        6945,
        6963,
        6968,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M580_VIEW_WALL_D2L_RIGHT,
        8503,
        8504,
        8512,
        8513,
        { 0, 20, 16, 31 },
        "DUNVIEW.C F0678:6837-6865; F0119:6900-7049; F0107 6968; F0128 8503-8513"
    };
    m->lanes[1] = (DM1_V1_D2L2D2R2F0107LanePc34){
        DM1_V1_D2L2_D2R2_F0107_SIDE_D2R2_PC34,
        "D2R2",
        DM1_C10_VIEW_SQUARE_D2R2,
        DM1_M605_VIEW_SQUARE_D2R,
        2,
        2,
        DM1_C708_ZONE_WALL_D2R2,
        DM1_C711_ZONE_WALL_D2R,
        6867,
        6895,
        7051,
        7224,
        7096,
        7114,
        7119,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        DM1_M581_VIEW_WALL_D2R_LEFT,
        8507,
        8508,
        8516,
        8517,
        { 208, 20, 16, 31 },
        "DUNVIEW.C F0679:6867-6895; F0120:7051-7224; F0107 7119; F0128 8507-8517"
    };
}

static void fill_calls(DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    m->calls[0] = (DM1_V1_D2L2D2R2F0107CallPc34){
        0,
        DM1_V1_D2L2_D2R2_F0107_SIDE_D2L2_PC34,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        "M551_RIGHT_WALL_ORNAMENT_ORDINAL",
        DM1_M580_VIEW_WALL_D2L_RIGHT,
        "M580_VIEW_WALL_D2L_RIGHT",
        6968,
        DM1_D2_WALL_ORNAMENT_COORDINATE_SET,
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
            DM1_D2_WALL_ORNAMENT_COORDINATE_SET, DM1_M580_VIEW_WALL_D2L_RIGHT),
        1,
        1,
        "DUNVIEW.C:6968 M551/M580; F0107 alcove boolean and C10 blit"
    };
    m->calls[1] = (DM1_V1_D2L2D2R2F0107CallPc34){
        1,
        DM1_V1_D2L2_D2R2_F0107_SIDE_D2R2_PC34,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        "M553_LEFT_WALL_ORNAMENT_ORDINAL",
        DM1_M581_VIEW_WALL_D2R_LEFT,
        "M581_VIEW_WALL_D2R_LEFT",
        7119,
        DM1_D2_WALL_ORNAMENT_COORDINATE_SET,
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
            DM1_D2_WALL_ORNAMENT_COORDINATE_SET, DM1_M581_VIEW_WALL_D2R_LEFT),
        1,
        1,
        "DUNVIEW.C:7119 M553/M581; F0107 alcove boolean and C10 blit"
    };
}

static void fill_steps(DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D2L2D2R2F0107StepPc34 steps[] = {
        { DM1_V1_D2L2_D2R2_F0107_STEP_F0128_D2L2_PC34, 0, 1,
          "F0128 updates and draws D2L2 guard", "DUNVIEW.C:8503-8504" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_F0128_D2R2_PC34, 1, 1,
          "F0128 updates and draws D2R2 guard", "DUNVIEW.C:8507-8508" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_F0119_D2L_BODY_PC34, 2, 1,
          "F0119 D2L carrier body", "DUNVIEW.C F0119:6900-7049" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_F0120_D2R_BODY_PC34, 3, 1,
          "F0120 D2R carrier body", "DUNVIEW.C F0120:7051-7224" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_D2L2_F0107_PC34, 4, 1,
          "D2L2 side wall ornament F0107", "DUNVIEW.C:6968 M551/M580" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_D2R2_F0107_PC34, 5, 1,
          "D2R2 side wall ornament F0107", "DUNVIEW.C:7119 M553/M581" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_F0108_BASELINE_PC34, 6, 1,
          "F0108 floor+ceiling baseline separation", "DUNVIEW.C F0108:3940-4011" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_ZONE_MATH_PC34, 7, 1,
          "C1004 + CoordinateSet * 15 + ViewWall", "DUNVIEW.C:3586-3587" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_C10_PC34, 8, 1,
          "C10 transparent wall-ornament blit", "DUNVIEW.C:3922; DEFS.H:2088" },
        { DM1_V1_D2L2_D2R2_F0107_STEP_SIBLING_NON_OVERLAP_PC34, 9, 1,
          "synthetic probe rejects sibling F0107 contracts", "D0/D1/D2/D3/CSB non-overlap guard" }
    };

    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_ordinals_pixels(DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u };
    static const uint8_t source[] = { 10u, 0x71u, 0x72u, 10u, 0x73u, 10u };
    size_t i;

    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].accepted_at_d2l2 = 1;
        m->ordinals[i].accepted_at_d2r2 = 1;
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H C0..C5 ornament ordinals; DUNGEON.C F0172 square aspect";
    }

    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].redmcsb_anchor =
            "DUNVIEW.C F0107:3922 C10_COLOR_FLESH transparent blit";
    }
}

static void fill_sibling_rejects(DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    m->sibling_rejects[0] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D0L/D0R F0107", 1, 1, 1, 1,
        0, -1, 1, 716, 717, -1, -1, 18, 74,
        "DUNVIEW.C F0125/F0126 D0 side routes"
    };
    m->sibling_rejects[1] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D1C F0107", 1, 1, 1, 1,
        1, 0, 0, 712, 712, 14, 14, 46, 90,
        "DUNVIEW.C F0124 D1C route"
    };
    m->sibling_rejects[2] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D1L/D1R F0107", 1, 1, 1, 1,
        1, -1, 1, 713, 714, 12, 13, 30, 88,
        "DUNVIEW.C F0122/F0123 D1 side routes"
    };
    m->sibling_rejects[3] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D2C F0107", 1, 1, 1, 1,
        2, 0, 0, 710, 711, 10, 10, 42, 64,
        "DUNVIEW.C F0121 D2C route"
    };
    m->sibling_rejects[4] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D2L/D2R F0107 front pair", 1, 1, 1, 1,
        2, -1, 1, 710, 711, 9, 11, 24, 55,
        "DUNVIEW.C F0119/F0120 front M582/M584 calls"
    };
    m->sibling_rejects[5] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D3L/D3R F0107", 1, 1, 1, 1,
        3, -1, 1, 705, 706, 2, 3, 16, 37,
        "DUNVIEW.C F0116/F0117 D3 side routes"
    };
    m->sibling_rejects[6] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "D3C F0107", 1, 1, 1, 1,
        3, 0, 0, 704, 704, 5, 5, 28, 25,
        "DUNVIEW.C F0118 D3C route"
    };
    m->sibling_rejects[7] = (DM1_V1_D2L2D2R2F0107SiblingRejectPc34){
        "CSB V1 D2L2/D2R2 F0107", 1, 1, 1, 1,
        2, -2, 2, 1707, 1708, 107, 108, 21, 29,
        "CSB-lineage Viewport.cpp custom background route, not ReDMCSB DUNVIEW.C"
    };
}

static int probe_collision_count(const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D2L2D2R2F0107RectPc34 sibling_rects[] = {
        { 4, 58, 18, 74 },
        { 86, 24, 46, 90 },
        { 23, 42, 30, 88 },
        { 54, 62, 42, 64 },
        { 54, 62, 24, 55 },
        { 74, 76, 16, 37 },
        { 98, 84, 28, 25 },
        { 38, 62, 21, 29 }
    };
    size_t lane_index;
    size_t sibling_index;
    int collisions = 0;

    if (!m) return -1;
    for (lane_index = 0; lane_index < DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34; ++lane_index) {
        for (sibling_index = 0;
             sibling_index < DM1_V1_D2L2_D2R2_F0107_SIBLING_REJECT_COUNT_PC34;
             ++sibling_index) {
            collisions += rects_overlap(&m->lanes[lane_index].probe_rect,
                                        &sibling_rects[sibling_index]) ? 1 : 0;
        }
    }
    return collisions;
}

bool dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D2L2_D2R2_F0107_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D2L2_D2R2_F0107_VIEWPORT_HEIGHT_PC34;
    out_model->c10_transparent_color = DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34;
    out_model->wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->wall_ornament_coordinate_set = DM1_D2_WALL_ORNAMENT_COORDINATE_SET;
    out_model->d2l2_wall_ornament_zone =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
            DM1_D2_WALL_ORNAMENT_COORDINATE_SET, DM1_M580_VIEW_WALL_D2L_RIGHT);
    out_model->d2r2_wall_ornament_zone =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_zone_pc34(
            DM1_D2_WALL_ORNAMENT_COORDINATE_SET, DM1_M581_VIEW_WALL_D2R_LEFT);
    out_model->d2l2_guard_wall_zone = DM1_C707_ZONE_WALL_D2L2;
    out_model->d2r2_guard_wall_zone = DM1_C708_ZONE_WALL_D2R2;
    out_model->m550_first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->m551_right_wall_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    out_model->m552_front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    out_model->m553_left_wall_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    out_model->f0128_d2l2_before_d2r2 = 1;
    out_model->f0128_d2r2_before_d2l_d2r_d2c = 1;
    out_model->f0119_f0120_body_pinned = 1;
    out_model->direct_f0107_call_count = 2;
    out_model->side_ornament_call_count = 2;
    out_model->front_ornament_call_count = 0;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_transparent_preserves_destination =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_blend_pixel_pc34(
            0x9au, DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34,
            DM1_V1_D2L2_D2R2_F0107_C10_COLOR_FLESH_PC34) == 0x9au;
    out_model->c0_to_c5_ordinals_pinned = 1;
    out_model->f0108_floor_ceiling_baseline_separate = 1;
    out_model->zone_math_pinned = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->redmcsb_c707_c708_zone_label_deviation_documented = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_lanes(out_model);
    fill_calls(out_model);
    fill_steps(out_model);
    fill_ordinals_pixels(out_model);
    fill_sibling_rejects(out_model);
    out_model->synthetic_probe_collision_count = probe_collision_count(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_width);
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_height);
    h = fnv1a_u32(h, (uint32_t)model->viewport_width);
    h = fnv1a_u32(h, (uint32_t)model->viewport_height);
    h = fnv1a_u32(h, (uint32_t)model->d2l2_wall_ornament_zone);
    h = fnv1a_u32(h, (uint32_t)model->d2r2_wall_ornament_zone);
    h = fnv1a_u32(h, (uint32_t)model->d2l2_guard_wall_zone);
    h = fnv1a_u32(h, (uint32_t)model->d2r2_guard_wall_zone);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d2l2_before_d2r2);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d2r2_before_d2l_d2r_d2c);
    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].guard_view_square);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].carrier_view_square);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].guard_wall_zone);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_line);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.x);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.y);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.width);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.height);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_CALL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->calls[i].aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->calls[i].view_wall);
        h = fnv1a_u32(h, (uint32_t)model->calls[i].zone);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].sensor_ordinal);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].accepted_at_d2l2);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].accepted_at_d2r2);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    for (i = 0; i < DM1_V1_D2L2_D2R2_F0107_SIBLING_REJECT_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].relative_depth);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].left_carrier_zone);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].right_carrier_zone);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].left_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].right_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].aspect_width);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].aspect_height);
    }
    h = fnv1a_u32(h, (uint32_t)model->synthetic_probe_collision_count);
    return h;
}

const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D2L2D2R2F0107LanePc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_lane_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34) ?
        NULL : &model->lanes[index];
}

const DM1_V1_D2L2D2R2F0107CallPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_call_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_F0107_CALL_COUNT_PC34) ?
        NULL : &model->calls[index];
}

const DM1_V1_D2L2D2R2F0107StepPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_F0107_STEP_COUNT_PC34) ?
        NULL : &model->steps[index];
}

const DM1_V1_D2L2D2R2F0107OrdinalPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_F0107_ORDINAL_COUNT_PC34) ?
        NULL : &model->ordinals[index];
}

const DM1_V1_D2L2D2R2F0107SiblingRejectPc34 *
dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_sibling_reject_at_pc34(size_t index)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D2L2_D2R2_F0107_SIBLING_REJECT_COUNT_PC34) ?
        NULL : &model->sibling_rejects[index];
}

int dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size)
{
    const DM1_V1_D2L2D2R2F0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_default_model_pc34();
    size_t lane_index;
    int writes = 0;

    if (!framebuffer ||
        framebuffer_size <
            (size_t)DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_WIDTH_PC34 *
            (size_t)DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_HEIGHT_PC34 ||
        !model) {
        return -1;
    }
    memset(framebuffer, 0, framebuffer_size);
    for (lane_index = 0; lane_index < DM1_V1_D2L2_D2R2_F0107_SIDE_COUNT_PC34; ++lane_index) {
        const DM1_V1_D2L2D2R2F0107RectPc34 *r = &model->lanes[lane_index].probe_rect;
        int y;
        for (y = r->y; y < r->y + r->height; ++y) {
            int x;
            for (x = r->x; x < r->x + r->width; ++x) {
                size_t offset =
                    (size_t)y * DM1_V1_D2L2_D2R2_F0107_FRAMEBUFFER_WIDTH_PC34 + (size_t)x;
                framebuffer[offset] = (uint8_t)(0x61u + lane_index);
                ++writes;
            }
        }
    }
    return writes;
}

const char *dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d2l2_d2r2_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
