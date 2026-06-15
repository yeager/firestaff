#include "csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat.h"

#include <string.h>

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D0L2 = 1,
    CSB_SIDE_D0R2 = 2,
    CSB_VIEW_SQUARE_D0L2 = 8,
    CSB_VIEW_SQUARE_D0R2 = 10,
    CSB_F0125 = 125,
    CSB_F0126 = 126,
    CSB_F0128_D0L_ORDER = 16,
    CSB_F0128_D0R_ORDER = 17,
    CSB_D0_DEPTH = 0,
    CSB_D0L2_LANE = -2,
    CSB_D0R2_LANE = 2,
    CSB_ZONE_WALL_D0L = 716,
    CSB_ZONE_WALL_D0R = 717,
    CSB_ZONE_DOOR_D0L2 = 3720,
    CSB_ZONE_DOOR_D0R2 = 3740,
    CSB_ORDER_REAR_D0L2 = 0x0028,
    CSB_ORDER_REAR_D0R2 = 0x0018,
    CSB_ORDER_FRONT_D0L2 = 0x0039,
    CSB_ORDER_FRONT_D0R2 = 0x0049,
    CSB_DOOR_STATE_OPEN = 0,
    CSB_DOOR_STATE_PARTLY_ONE = 1,
    CSB_DOOR_STATE_PARTLY_TWO = 2,
    CSB_DOOR_STATE_PARTLY_THREE = 3,
    CSB_DOOR_STATE_CLOSED = 4,
    CSB_DOOR_STATE_DESTROYED = 5,
    CSB_C6_UNKNOWN = 6,
    CSB_SECOND_HALF_OFFSET = 3,
    CSB_MASK0X4000 = 0x4000,
    CSB_MASK0X8000 = 0x8000,
    CSB_C10_COLOR_FLESH = 10
};

static const char s_source_evidence[] =
    "pass719 source_locked_contract_only=1; no_real_asset_bitmap_parity=1; "
    "no_game_data_load=1. ReDMCSB DUNVIEW.C F0111:4218-4339 anchors "
    "the partly-open door-front branch: 4308 decrements state, 4311-4313 "
    "selects LeftHorizontal/RightHorizontal, 4317-4325 does zone math and "
    "MASK 0x4000, and 4334 performs the C10 transparent F0791 blit. "
    "ReDMCSB DUNVIEW.C F0104:3113-3156 and F0105:3185-3247 anchor native "
    "and flipped C10 blit contracts; D0R uses the flipped horizontal lane. "
    "ReDMCSB DUNVIEW.C F0107:3502-3938 is wall ornament drawing and is "
    "kept out of this door composition; F0108:3940-4011 is floor ornament "
    "drawing and MASK 0x8000 footprint recursion, also kept out of the "
    "door band. ReDMCSB DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,"
    "5668-5671 anchors thing-pass cell ordering around the door. "
    "ReDMCSB DUNVIEW.C F0128:8318-8486 and 8536-8541 anchors D0L/D0R "
    "dispatch plus mirror flip setup. ReDMCSB DUNGEON.C F0163:1769-1838, "
    "F0164:1840-1905, and F0172:2466-2523 prove draw-only square aspect "
    "inputs without caller thing-list mutation. ReDMCSB DEFS.H:2088 C10, "
    "2596-2611 view squares including 8/10, 2662 and 2668-2677 cell orders, "
    "4045-4046 C705/C706, 4139-4153 cell-order zone band, and partly-open "
    "ordinals at 4045-4046/4139-4153 are carried as source-lock anchors. "
    "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915,1930-1944 "
    "anchor CSB partly-open door composition and D0 return-only contrast; "
    "Viewport.cpp:6507-6548 anchors CustomBackgrounds ApplyDecoration after "
    "floor/ceiling and before door-front composition.";

#define D0_SPEC(side_value, route, square, func, order, lane, flip, wall_zone, \
                door_zone, rear_order, front_order, left_name, right_name) \
    { \
        side_value, route, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, square, \
        order, CSB_D0_DEPTH, lane, func, flip, wall_zone, door_zone, \
        rear_order, front_order, CSB_DOOR_STATE_OPEN, \
        CSB_DOOR_STATE_PARTLY_ONE, CSB_DOOR_STATE_PARTLY_TWO, \
        CSB_DOOR_STATE_PARTLY_THREE, CSB_DOOR_STATE_CLOSED, \
        CSB_DOOR_STATE_DESTROYED, 4, CSB_C6_UNKNOWN, \
        CSB_SECOND_HALF_OFFSET, CSB_MASK0X4000, CSB_C10_COLOR_FLESH, \
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, \
        CSB_PRESENT, left_name, right_name, \
        "ReDMCSB DUNVIEW.C F0111:4218-4339 partly-open door-front blit", \
        "ReDMCSB DUNVIEW.C F0104:3113-3156 native C10 blit", \
        "ReDMCSB DUNVIEW.C F0105:3185-3247 flipped C10 blit", \
        "ReDMCSB DUNVIEW.C F0107:3502-3938 wall ornament keepout", \
        "ReDMCSB DUNVIEW.C F0108:3940-4011 floor ornament/footprint keepout", \
        "ReDMCSB DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671", \
        "ReDMCSB DUNVIEW.C F0128:8318-8486,8536-8541 mirror/D0 dispatch", \
        "ReDMCSB DUNGEON.C F0163:1769-1838; F0164:1840-1905; F0172:2466-2523", \
        "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153", \
        "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915,1930-1944", \
        "CSB-lineage Viewport.cpp:6507-6548 ApplyDecoration hook" \
    }

static const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 s_specs[] = {
    D0_SPEC(CSB_SIDE_D0L2,
            "D0L2 F0111 partly-open door after CustomBackgrounds mask",
            CSB_VIEW_SQUARE_D0L2,
            CSB_F0125,
            CSB_F0128_D0L_ORDER,
            CSB_D0L2_LANE,
            CSB_ABSENT,
            CSB_ZONE_WALL_D0L,
            CSB_ZONE_DOOR_D0L2,
            CSB_ORDER_REAR_D0L2,
            CSB_ORDER_FRONT_D0L2,
            "G0185_s_Graphic558_Frames_Door_D0L2.LeftHorizontal[state-1]",
            "G0185_s_Graphic558_Frames_Door_D0L2.RightHorizontal[state-1]"),
    D0_SPEC(CSB_SIDE_D0R2,
            "D0R2 F0111 partly-open door after CustomBackgrounds mask",
            CSB_VIEW_SQUARE_D0R2,
            CSB_F0126,
            CSB_F0128_D0R_ORDER,
            CSB_D0R2_LANE,
            CSB_PRESENT,
            CSB_ZONE_WALL_D0R,
            CSB_ZONE_DOOR_D0R2,
            CSB_ORDER_REAR_D0R2,
            CSB_ORDER_FRONT_D0R2,
            "G0187_s_Graphic558_Frames_Door_D0R2.LeftHorizontal[state-1]",
            "G0187_s_Graphic558_Frames_Door_D0R2.RightHorizontal[state-1]")
};

#undef D0_SPEC

static const CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34 s_order[] = {
    CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_MASK_AFTER_FLOOR_CEILING_PC34,
    CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_ROOM_BITMAP_PC34,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0115_REAR_OBJECTS_PC34,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_FIRST_HALF_PC34,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_SECOND_HALF_PC34,
    CSB_V1_D0L2_D0R2_F0111_STEP_F0115_FRONT_OBJECTS_PC34
};

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

size_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(int side)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34;
    if (door_state == spec->open_state) {
        return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_OPEN_PC34;
    }
    if (door_state == spec->closed_state) {
        return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_CLOSED_PC34;
    }
    if (door_state == spec->destroyed_state) {
        return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_DESTROYED_PC34;
    }
    if (door_state >= spec->partly_open_state_one &&
        door_state <= spec->partly_open_state_three) {
        return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    }
    return CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_open_fraction_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    const int branch =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            spec, door_state);
    if (!spec) return -1;
    if (branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_OPEN_PC34) return 0;
    if (branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return door_state;
    }
    if (branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_CLOSED_PC34) {
        return spec->open_fraction_denominator;
    }
    return -1;
}

const char *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half)
{
    if (csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }
    return right_half ? spec->right_horizontal_frame_bitmap :
                        spec->left_horizontal_frame_bitmap;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (!horizontal_door) return -1;
    if (csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    return spec->door_zone_base + door_state + spec->first_half_zone_offset;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    const int branch =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            spec, door_state);
    if (!spec) return -1;
    if (branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34 ||
        branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_OPEN_PC34) {
        return -1;
    }
    if (branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_CLOSED_PC34 ||
        branch == CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_DESTROYED_PC34) {
        return spec->door_zone_base;
    }
    if (!horizontal_door) return spec->door_zone_base + door_state;
    return spec->door_zone_base + door_state +
           (spec->second_half_zone_offset | spec->second_half_zone_mask);
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(
    unsigned int order,
    int ordinal)
{
    unsigned int shift;
    unsigned int cell;

    if (ordinal < 0 || ordinal > 3) return -1;
    shift = (unsigned int)ordinal * 4u;
    cell = (order >> shift) & 0x0fu;
    if (cell == 0u || cell == 8u || cell == 9u) return -1;
    return (int)cell - 1;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int source_width,
    int x)
{
    if (!spec || source_width <= 0 || x < 0 || x >= source_width) return -1;
    if (spec->d0r_uses_horizontal_flip) return source_width - 1 - x;
    return x;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int wall_ornament_ordinal,
    int door_composition_active)
{
    if (!spec || wall_ornament_ordinal <= 0) return 0;
    if (door_composition_active && spec->wall_ornament_keepout) return 0;
    return 1;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int floor_ornament_ordinal,
    int door_composition_active,
    int *out_footprint_recursions)
{
    int recursions = 0;
    if (floor_ornament_ordinal & CSB_MASK0X8000) {
        recursions = 1;
        floor_ornament_ordinal &= ~CSB_MASK0X8000;
    }
    if (out_footprint_recursions) *out_footprint_recursions = recursions;
    if (!spec || floor_ornament_ordinal <= 0) return 0;
    if (door_composition_active && spec->floor_ornament_keepout) return 0;
    return 1;
}

size_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_order_pc34(
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34 *out_steps,
    size_t out_capacity)
{
    const size_t count = sizeof(s_order) / sizeof(s_order[0]);
    if (out_steps && out_capacity > 0) {
        const size_t copy_count = out_capacity < count ? out_capacity : count;
        memcpy(out_steps, s_order, copy_count * sizeof(s_order[0]));
    }
    return count;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34 *out_result)
{
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34 result;
    uint32_t hash = 2166136261u;

    memset(&result, 0, sizeof(result));
    result.deterministic_hash =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34();

    if (!spec || !source || !destination || !out_result) {
        if (out_result) {
            result.mutation_rejections = 1;
            *out_result = result;
        }
        return -1;
    }
    if (source_width <= 0 || source_height <= 0 ||
        source_stride < source_width ||
        destination_width <= 0 || destination_height <= 0 ||
        destination_stride < destination_width ||
        destination_width < source_width ||
        destination_height < source_height) {
        result.row_guard_rejections = 1;
        result.mutation_rejections = 1;
        *out_result = result;
        return -1;
    }
    if (csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        *out_result = result;
        return 0;
    }

    for (int y = 0; y < source_height; ++y) {
        for (int x = 0; x < source_width; ++x) {
            const int sx =
                csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(
                    spec, source_width, x);
            const uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)spec->c10_transparent_color) {
                ++result.c10_skipped_pixels;
                continue;
            }
            destination[(y * destination_stride) + x] = pixel;
            ++result.copied_pixels;
            if (x == 0) ++result.left_edge_writes;
            if (x == source_width - 1) ++result.right_edge_writes;
            hash = hash_u32(hash, (uint32_t)pixel);
            hash = hash_u32(hash, (uint32_t)((y << 16) | x));
        }
    }

    result.ok = 1;
    result.deterministic_hash = hash;
    *out_result = result;
    return result.copied_pixels;
}

uint32_t csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34(void)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i) {
        hash = hash_u32(hash, (uint32_t)s_specs[i].side);
        hash = hash_u32(hash, (uint32_t)s_specs[i].view_square);
        hash = hash_u32(hash, (uint32_t)s_specs[i].door_zone_base);
        hash = hash_u32(hash, (uint32_t)s_specs[i].rear_cell_order);
        hash = hash_u32(hash, (uint32_t)s_specs[i].front_cell_order);
    }
    hash = hash_u32(hash, (uint32_t)CSB_C10_COLOR_FLESH);
    hash = hash_u32(hash, (uint32_t)CSB_MASK0X8000);
    hash = hash_u32(hash, (uint32_t)CSB_MASK0X4000);
    return hash;
}

int csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorProbePc34 *out_probe)
{
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_SIDE_D0L2);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_SIDE_D0R2);
    const uint8_t source[8] = { 1, 10, 2, 3, 4, 5, 10, 6 };
    uint8_t destination[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34 blit;
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34 order[6];

    if (!out_probe || !left || !right) return -1;
    memset(out_probe, 0, sizeof(*out_probe));
    if (csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
            right, CSB_DOOR_STATE_PARTLY_TWO, source, 4, 2, 4, destination,
            4, 2, 4, &blit) < 0) {
        return -1;
    }
    (void)csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_order_pc34(order, 6);

    out_probe->route_count =
        (int)csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34();
    out_probe->d0r_flip_ok =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(
            right, 4, 0) == 3;
    out_probe->partly_open_gate_ok =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO) ==
        CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    out_probe->custom_backgrounds_depth_ok =
        order[0] == CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_MASK_AFTER_FLOOR_CEILING_PC34 &&
        order[3] == CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_FIRST_HALF_PC34;
    out_probe->wall_keepout_ok =
        !csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(
            left, 1, CSB_PRESENT);
    out_probe->floor_keepout_ok =
        !csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
            left, CSB_MASK0X8000 | 1, CSB_PRESENT,
            &out_probe->row_guard_rejections);
    out_probe->first_half_zone =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_first_half_zone_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->second_half_zone =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->copied_pixels = blit.copied_pixels;
    out_probe->c10_skipped_pixels = blit.c10_skipped_pixels;
    out_probe->mutation_rejections = blit.mutation_rejections;
    out_probe->deterministic_hash =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34();
    return 0;
}

const char *
csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
