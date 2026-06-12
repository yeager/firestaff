#include "firestaff/dm1/v1/viewport/d1l_d1r_f0107_wall_ornament_pc34_compat.h"

#include <string.h>

enum {
    DM1_M607_VIEW_SQUARE_D1L = 4,
    DM1_M608_VIEW_SQUARE_D1R = 5,
    DM1_C713_ZONE_WALL_D1L = 713,
    DM1_C714_ZONE_WALL_D1R = 714,
    DM1_M594_VIEW_FLOOR_D1L = 8,
    DM1_M596_VIEW_FLOOR_D1R = 10,
    DM1_M630_ZONE_DOOR_D1L = 3780,
    DM1_M632_ZONE_DOOR_D1R = 3800,
    DM1_C1004_ZONE_WALL_ORNAMENT = 1004,
    DM1_WALL_ORNAMENT_ZONE_STRIDE = 15,
    DM1_D1_WALL_ORNAMENT_COORDINATE_SET = 2,
    DM1_M550_FIRST_THING_SLOT = 2,
    DM1_M551_RIGHT_WALL_ORNAMENT_SLOT = 4,
    DM1_M552_FRONT_WALL_ORNAMENT_SLOT = 5,
    DM1_M553_LEFT_WALL_ORNAMENT_SLOT = 6,
    DM1_M585_VIEW_WALL_D1L_RIGHT = 12,
    DM1_M586_VIEW_WALL_D1R_LEFT = 13,
    DM1_D1L_CORRIDOR_ORDER = 0x0032,
    DM1_D1R_CORRIDOR_ORDER = 0x0041,
    DM1_D1L_DOOR_SIDE_ORDER = 0x0032,
    DM1_D1R_DOOR_SIDE_ORDER = 0x0041,
    DM1_D1L_DOOR_PASS1_ORDER = 0x0028,
    DM1_D1R_DOOR_PASS1_ORDER = 0x0018,
    DM1_D1L_DOOR_PASS2_ORDER = 0x0039,
    DM1_D1R_DOOR_PASS2_ORDER = 0x0049
};

/*
 * ReDMCSB source lock:
 * - DUNVIEW.C F0107:3502-3938 owns wall-ornament dispatch, C10
 *   transparency, C1004 + CoordinateSet * 15 + ViewWall zone math, and
 *   alcove return.
 * - The pass contract names DUNVIEW.C F0114:6651-6740 and F0115:6742-6886
 *   for D1 side-wall callers. In the local ReDMCSB WIP20210206 source those
 *   D1L/D1R callers are F0122:7391-7557 and F0123:7559-7725, with F0107 at
 *   7459 and 7627.
 * - DUNVIEW.C F0128:8503-8517 is the nearby dispatch tail; the current local
 *   file draws D1L, then D1R, then D1C before D0L/D0R/D0C at 8524-8542.
 * - DUNVIEW.C F0108:3940-4011, DUNGEON.C F0163:1769-1838,
 *   F0164:1840-1905, F0172:2466-2523, and DEFS.H C10/C0..C5/C713/C714
 *   complete this contract-only, asset-free slice.
 */
static const char s_source_evidence[] =
    "ReDMCSB source-lock: DUNVIEW.C F0107:3502-3938 decrements non-zero "
    "wall-ornament ordinals at 3571-3575, computes C1004_ZONE_WALL_ORNAMENT "
    "+ CoordinateSet * C15 + ViewWall at 3586-3587, classifies alcoves at "
    "3589, draws with C10_COLOR_FLESH at 3922, and returns the alcove flag "
    "at 3933. The pass contract names DUNVIEW.C F0114:6651-6740 and "
    "F0115:6742-6886 as the D1L/D1R wall-body callers; the local "
    "ReDMCSB WIP20210206 source exposes those D1 side callers as "
    "F0122:7391-7557 and F0123:7559-7725. D1L draws C713 at 7454 and calls "
    "F0107 at 7459 with M551_RIGHT_WALL_ORNAMENT_ORDINAL and "
    "M585_VIEW_WALL_D1L_RIGHT. D1R draws C714 at 7622 and calls F0107 at "
    "7627 with M553_LEFT_WALL_ORNAMENT_ORDINAL and M586_VIEW_WALL_D1R_LEFT. "
    "DUNVIEW.C F0128:8503-8517 is the requested dispatch anchor; the local "
    "tail continues with D1L at 8524-8525, D1R at 8528-8529, D1C at "
    "8532-8533, D0L at 8536-8537, D0R at 8540-8541, and D0C at 8542. "
    "DUNVIEW.C F0108:3940-4011 pins the floor+ceiling+ornament baseline "
    "called from the D1L/D1R non-wall paths at 7493/7525 and 7661/7693. "
    "DUNGEON.C F0163:1769-1838 and F0164:1840-1905 anchor thing-list "
    "mutation, while F0172:2466-2523 populates the square-aspect slots "
    "that feed M551/M553. DEFS.H:2088 defines C10_COLOR_FLESH; "
    "DEFS.H:2548-2559 defines C0..C5/M550/M551/M552/M553; "
    "DEFS.H:2596-2611 defines M607/M608; DEFS.H:2696-2711 defines "
    "M585/M586; DEFS.H:4053-4054 defines C713/C714; DEFS.H:4221-4225 "
    "defines the C1004/C1500/C1950/C2000 zone bases.";

static const char s_disjointness_note[] =
    "D1L/D1R F0107 wall-ornament contract only. It covers the unique "
    "middle-depth side pair at relative cells (1,-1) and (1,+1), wall "
    "carriers C713/C714, view-wall ordinals M585/M586, and the direct "
    "M551/M553 F0107 calls. The synthetic framebuffer probe rejects the "
    "D0L/D0R, D1C, D2L/D2R, and D3L/D3R F0107 contracts by different cell "
    "position, different carrier zones including C705/C706 for D3, different "
    "view-wall ordinals, and different probe aspect ratio. It does not read "
    "GRAPHICS.DAT and makes no original DOS pixel parity claim.";

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static int rects_overlap(const DM1_V1_D1LD1RF0107RectPc34 *a,
                         const DM1_V1_D1LD1RF0107RectPc34 *b)
{
    if (!a || !b) return 0;
    return a->x < b->x + b->width &&
           a->x + a->width > b->x &&
           a->y < b->y + b->height &&
           a->y + a->height > b->y;
}

int dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_zone_pc34(
    int coordinate_set,
    int view_wall)
{
    if (coordinate_set < 0 || view_wall < 0) return -1;
    return DM1_C1004_ZONE_WALL_ORNAMENT +
           coordinate_set * DM1_WALL_ORNAMENT_ZONE_STRIDE + view_wall;
}

uint8_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    return source_pixel == transparent_color ? destination_pixel : source_pixel;
}

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(
    int wall_ornament_ordinal,
    bool dungeon_classifies_alcove)
{
    return wall_ornament_ordinal != 0 && dungeon_classifies_alcove;
}

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_accepts_sensor_ordinal_pc34(
    int side_index,
    int ornament_index_c0_to_c5)
{
    return side_index >= 0 &&
           side_index < DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34 &&
           ornament_index_c0_to_c5 >= 0 &&
           ornament_index_c0_to_c5 < DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34;
}

static void fill_lanes(DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m)
{
    m->lanes[0] = (DM1_V1_D1LD1RF0107LanePc34){
        DM1_V1_D1L_D1R_F0107_SIDE_D1L_PC34,
        "D1L",
        DM1_M607_VIEW_SQUARE_D1L,
        1,
        -1,
        DM1_C713_ZONE_WALL_D1L,
        DM1_M594_VIEW_FLOOR_D1L,
        DM1_M630_ZONE_DOOR_D1L,
        7391,
        7557,
        7436,
        7454,
        7459,
        DM1_M551_RIGHT_WALL_ORNAMENT_SLOT,
        DM1_M585_VIEW_WALL_D1L_RIGHT,
        8524,
        8525,
        DM1_D1L_CORRIDOR_ORDER,
        DM1_D1L_DOOR_SIDE_ORDER,
        DM1_D1L_DOOR_PASS1_ORDER,
        DM1_D1L_DOOR_PASS2_ORDER,
        7525,
        7536,
        { 23, 42, 30, 88 },
        "DUNVIEW.C F0122:7391-7557; F0107 at 7459; F0128 D1L 8524-8525"
    };
    m->lanes[1] = (DM1_V1_D1LD1RF0107LanePc34){
        DM1_V1_D1L_D1R_F0107_SIDE_D1R_PC34,
        "D1R",
        DM1_M608_VIEW_SQUARE_D1R,
        1,
        1,
        DM1_C714_ZONE_WALL_D1R,
        DM1_M596_VIEW_FLOOR_D1R,
        DM1_M632_ZONE_DOOR_D1R,
        7559,
        7725,
        7604,
        7622,
        7627,
        DM1_M553_LEFT_WALL_ORNAMENT_SLOT,
        DM1_M586_VIEW_WALL_D1R_LEFT,
        8528,
        8529,
        DM1_D1R_CORRIDOR_ORDER,
        DM1_D1R_DOOR_SIDE_ORDER,
        DM1_D1R_DOOR_PASS1_ORDER,
        DM1_D1R_DOOR_PASS2_ORDER,
        7693,
        7704,
        { 172, 42, 34, 88 },
        "DUNVIEW.C F0123:7559-7725; F0107 at 7627; F0128 D1R 8528-8529"
    };
}

static void fill_steps(DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D1LD1RF0107StepPc34 steps[] = {
        { DM1_V1_D1L_D1R_F0107_STEP_F0128_D1L_PC34, 0, 1,
          "F0128 updates and draws D1L", "DUNVIEW.C F0128:8503-8517; local 8524-8525" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0128_D1R_PC34, 1, 1,
          "F0128 updates and draws D1R after D1L", "DUNVIEW.C F0128:8503-8517; local 8528-8529" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0122_D1L_BODY_PC34, 2, 1,
          "D1L wall body", "DUNVIEW.C F0122:7391-7557; pass label F0114:6651-6740" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0123_D1R_BODY_PC34, 3, 1,
          "D1R wall body", "DUNVIEW.C F0123:7559-7725; pass label F0115:6742-6886" },
        { DM1_V1_D1L_D1R_F0107_STEP_D1L_F0107_PC34, 4, 1,
          "D1L direct side wall ornament", "DUNVIEW.C:7459 M551/M585" },
        { DM1_V1_D1L_D1R_F0107_STEP_D1R_F0107_PC34, 5, 1,
          "D1R direct side wall ornament", "DUNVIEW.C:7627 M553/M586" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0108_BASELINE_PC34, 6, 1,
          "F0108 floor+ceiling+ornament baseline", "DUNVIEW.C F0108:3940-4011; D1L 7525; D1R 7693" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0115_CELL_ORDER_PC34, 7, 1,
          "F0115 side-lane cell-order pass", "DUNVIEW.C F0115:4547-4581; D1L 7536; D1R 7704" },
        { DM1_V1_D1L_D1R_F0107_STEP_F0107_C10_PC34, 8, 1,
          "C10 transparent wall-ornament blit", "DUNVIEW.C F0107:3922; DEFS.H:2088" },
        { DM1_V1_D1L_D1R_F0107_STEP_SIBLING_NON_OVERLAP_PC34, 9, 1,
          "synthetic probe rejects sibling F0107 contracts", "D0/D1C/D2/D3 sibling non-overlap guard" }
    };

    memcpy(m->steps, steps, sizeof(steps));
}

static void fill_ordinals_pixels(DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m)
{
    static const uint8_t before[] = { 0x21u, 0x22u, 0x23u, 0x24u, 0x25u, 0x26u };
    static const uint8_t source[] = { 10u, 0x61u, 0x62u, 10u, 0x63u, 10u };
    size_t i;

    for (i = 0; i < DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34; ++i) {
        m->ordinals[i].ordinal_index_c0_to_c5 = (int)i;
        m->ordinals[i].sensor_ordinal = (int)i + 1;
        m->ordinals[i].accepted_at_d1l = 1;
        m->ordinals[i].accepted_at_d1r = 1;
        m->ordinals[i].redmcsb_anchor =
            "DEFS.H C0..C5 ornament ordinals; DUNGEON.C F0172 square aspect";
    }

    for (i = 0; i < DM1_V1_D1L_D1R_F0107_PIXEL_COUNT_PC34; ++i) {
        m->pixels[i].before = before[i];
        m->pixels[i].source = source[i];
        m->pixels[i].after =
            dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(
                before[i], source[i], DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34);
        m->pixels[i].transparent_skip =
            source[i] == DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].writes_pixel =
            source[i] != DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34;
        m->pixels[i].redmcsb_anchor =
            "DUNVIEW.C F0107:3922 C10_COLOR_FLESH transparent blit";
    }
}

static void fill_sibling_rejects(DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m)
{
    m->sibling_rejects[0] = (DM1_V1_D1LD1RF0107SiblingRejectPc34){
        "D0L/D0R F0107",
        1, 1, 1, 1,
        0, -1, 1, 716, 717, -1, -1, 18, 74,
        "DUNVIEW.C F0125/F0126 D0 side routes; no direct D1 M585/M586 F0107"
    };
    m->sibling_rejects[1] = (DM1_V1_D1LD1RF0107SiblingRejectPc34){
        "D1C F0107",
        1, 1, 1, 1,
        1, 0, 0, 712, 712, 14, 14, 46, 90,
        "DUNVIEW.C F0124:7727-7924 D1C uses M552/M587 and C712"
    };
    m->sibling_rejects[2] = (DM1_V1_D1LD1RF0107SiblingRejectPc34){
        "D2L/D2R F0107",
        1, 1, 1, 1,
        2, -1, 1, 710, 711, 7, 8, 24, 55,
        "DUNVIEW.C F0119/F0120 D2 uses C710/C711 and M580/M581/M582/M584"
    };
    m->sibling_rejects[3] = (DM1_V1_D1LD1RF0107SiblingRejectPc34){
        "D3L/D3R F0107",
        1, 1, 1, 1,
        3, -1, 1, 705, 706, 2, 3, 16, 37,
        "DUNVIEW.C F0116/F0117 D3 uses C705/C706 and M575/M576/M577/M579"
    };
}

static int probe_collision_count(const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *m)
{
    static const DM1_V1_D1LD1RF0107RectPc34 sibling_rects[] = {
        { 4, 58, 18, 74 },
        { 86, 24, 46, 90 },
        { 54, 62, 24, 55 },
        { 74, 76, 16, 37 }
    };
    size_t lane_index;
    size_t sibling_index;
    int collisions = 0;

    if (!m) return -1;
    for (lane_index = 0; lane_index < DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34; ++lane_index) {
        for (sibling_index = 0;
             sibling_index < DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34;
             ++sibling_index) {
            collisions += rects_overlap(&m->lanes[lane_index].probe_rect,
                                        &sibling_rects[sibling_index]) ? 1 : 0;
        }
    }
    return collisions;
}

bool dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_builder_pc34(
    DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *out_model)
{
    if (!out_model) return false;
    memset(out_model, 0, sizeof(*out_model));

    out_model->framebuffer_width = DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34;
    out_model->framebuffer_height = DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_HEIGHT_PC34;
    out_model->viewport_width = DM1_V1_D1L_D1R_F0107_VIEWPORT_WIDTH_PC34;
    out_model->viewport_height = DM1_V1_D1L_D1R_F0107_VIEWPORT_HEIGHT_PC34;
    out_model->viewport_x = 0;
    out_model->viewport_y = 0;
    out_model->c10_transparent_color = DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34;
    out_model->wall_ornament_zone_base = DM1_C1004_ZONE_WALL_ORNAMENT;
    out_model->wall_ornament_zone_stride = DM1_WALL_ORNAMENT_ZONE_STRIDE;
    out_model->wall_ornament_coordinate_set = DM1_D1_WALL_ORNAMENT_COORDINATE_SET;
    out_model->d1l_wall_ornament_zone =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_zone_pc34(
            DM1_D1_WALL_ORNAMENT_COORDINATE_SET, DM1_M585_VIEW_WALL_D1L_RIGHT);
    out_model->d1r_wall_ornament_zone =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_zone_pc34(
            DM1_D1_WALL_ORNAMENT_COORDINATE_SET, DM1_M586_VIEW_WALL_D1R_LEFT);
    out_model->m550_first_thing_slot = DM1_M550_FIRST_THING_SLOT;
    out_model->m551_right_wall_ornament_slot = DM1_M551_RIGHT_WALL_ORNAMENT_SLOT;
    out_model->m552_front_wall_ornament_slot = DM1_M552_FRONT_WALL_ORNAMENT_SLOT;
    out_model->m553_left_wall_ornament_slot = DM1_M553_LEFT_WALL_ORNAMENT_SLOT;
    out_model->f0128_d1l_before_d1r = 1;
    out_model->f0128_d1r_before_d1c = 1;
    out_model->direct_f0107_call_count = 2;
    out_model->side_ornament_call_count = 2;
    out_model->front_ornament_call_count = 0;
    out_model->f0107_zero_ordinal_returns_false =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(0, true) ? 0 : 1;
    out_model->f0107_non_alcove_returns_false =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(3, false) ? 0 : 1;
    out_model->f0107_alcove_returns_true =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_returns_alcove_pc34(3, true) ? 1 : 0;
    out_model->f0107_blit_uses_c10 = 1;
    out_model->c10_transparent_preserves_destination =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_blend_pixel_pc34(
            0x9au, DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34,
            DM1_V1_D1L_D1R_F0107_C10_COLOR_FLESH_PC34) == 0x9au;
    out_model->c0_to_c5_ordinals_pinned = 1;
    out_model->f0108_baseline_pinned = 1;
    out_model->f0115_cell_order_pinned = 1;
    out_model->source_locked_contract_only = 1;
    out_model->no_original_dos_pixel_parity = 1;
    out_model->no_graphics_dat_reads = 1;
    out_model->source_evidence = s_source_evidence;
    out_model->disjointness_note = s_disjointness_note;

    fill_lanes(out_model);
    fill_steps(out_model);
    fill_ordinals_pixels(out_model);
    fill_sibling_rejects(out_model);
    out_model->synthetic_probe_collision_count = probe_collision_count(out_model);
    out_model->deterministic_hash =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_hash_model_pc34(out_model);
    return true;
}

uint32_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_hash_model_pc34(
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model)
{
    uint32_t h = 2166136261u;
    size_t i;

    if (!model) return 0u;
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_width);
    h = fnv1a_u32(h, (uint32_t)model->framebuffer_height);
    h = fnv1a_u32(h, (uint32_t)model->viewport_width);
    h = fnv1a_u32(h, (uint32_t)model->viewport_height);
    h = fnv1a_u32(h, (uint32_t)model->d1l_wall_ornament_zone);
    h = fnv1a_u32(h, (uint32_t)model->d1r_wall_ornament_zone);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d1l_before_d1r);
    h = fnv1a_u32(h, (uint32_t)model->f0128_d1r_before_d1c);
    for (i = 0; i < DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].view_square);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].wall_zone);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_line);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_aspect_slot);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].f0107_view_wall);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.x);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.y);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.width);
        h = fnv1a_u32(h, (uint32_t)model->lanes[i].probe_rect.height);
    }
    for (i = 0; i < DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].sensor_ordinal);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].accepted_at_d1l);
        h = fnv1a_u32(h, (uint32_t)model->ordinals[i].accepted_at_d1r);
    }
    for (i = 0; i < DM1_V1_D1L_D1R_F0107_PIXEL_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].before);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].source);
        h = fnv1a_u32(h, (uint32_t)model->pixels[i].after);
    }
    for (i = 0; i < DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34; ++i) {
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].relative_depth);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].left_carrier_zone);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].right_carrier_zone);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].aspect_width);
        h = fnv1a_u32(h, (uint32_t)model->sibling_rejects[i].aspect_height);
    }
    h = fnv1a_u32(h, (uint32_t)model->synthetic_probe_collision_count);
    return h;
}

const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34(void)
{
    static DM1_V1_D1LD1RF0107WallOrnamentModelPc34 s_model;
    static int s_init;

    if (!s_init) {
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_builder_pc34(&s_model);
        s_init = 1;
    }
    return &s_model;
}

uint32_t dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_deterministic_hash_pc34(void)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    return model ? model->deterministic_hash : 0u;
}

const DM1_V1_D1LD1RF0107LanePc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_lane_at_pc34(size_t index)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34) ? NULL : &model->lanes[index];
}

const DM1_V1_D1LD1RF0107StepPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_step_at_pc34(size_t index)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D1L_D1R_F0107_STEP_COUNT_PC34) ? NULL : &model->steps[index];
}

const DM1_V1_D1LD1RF0107OrdinalPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_ordinal_at_pc34(size_t index)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D1L_D1R_F0107_ORDINAL_COUNT_PC34) ? NULL : &model->ordinals[index];
}

const DM1_V1_D1LD1RF0107SiblingRejectPc34 *
dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_sibling_reject_at_pc34(size_t index)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    return (!model || index >= DM1_V1_D1L_D1R_F0107_SIBLING_REJECT_COUNT_PC34) ?
        NULL : &model->sibling_rejects[index];
}

int dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_render_probe_pc34(
    uint8_t *framebuffer,
    size_t framebuffer_size)
{
    const DM1_V1_D1LD1RF0107WallOrnamentModelPc34 *model =
        dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_default_model_pc34();
    size_t lane_index;
    int writes = 0;

    if (!framebuffer ||
        framebuffer_size <
            (size_t)DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 *
            (size_t)DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_HEIGHT_PC34 ||
        !model) {
        return -1;
    }
    memset(framebuffer, 0, framebuffer_size);
    for (lane_index = 0; lane_index < DM1_V1_D1L_D1R_F0107_SIDE_COUNT_PC34; ++lane_index) {
        const DM1_V1_D1LD1RF0107RectPc34 *r = &model->lanes[lane_index].probe_rect;
        int y;
        for (y = r->y; y < r->y + r->height; ++y) {
            int x;
            for (x = r->x; x < r->x + r->width; ++x) {
                size_t offset =
                    (size_t)y * DM1_V1_D1L_D1R_F0107_FRAMEBUFFER_WIDTH_PC34 + (size_t)x;
                framebuffer[offset] = (uint8_t)(0x51u + lane_index);
                ++writes;
            }
        }
    }
    return writes;
}

const char *dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const char *dm1_v1_viewport_d1l_d1r_f0107_wall_ornament_disjointness_note_pc34(void)
{
    return s_disjointness_note;
}
