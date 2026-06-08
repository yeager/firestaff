#include "csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_SIDE_D1L2 = 1,
    CSB_SIDE_D1R2 = 2,
    CSB_VIEW_SQUARE_D1L = 4,       /* ReDMCSB: DEFS.H line 2600 M607_VIEW_SQUARE_D1L. */
    CSB_VIEW_SQUARE_D1R = 5,       /* ReDMCSB: DEFS.H line 2601 M608_VIEW_SQUARE_D1R. */
    CSB_F0122 = 122,               /* ReDMCSB: DUNVIEW.C F0122 lines 7391-7557. */
    CSB_F0123 = 123,               /* ReDMCSB: DUNVIEW.C F0123 lines 7559-7725. */
    CSB_F0128_D1L_ORDER = 13,      /* ReDMCSB: DUNVIEW.C F0128 lines 8524-8525. */
    CSB_F0128_D1R_ORDER = 14,      /* ReDMCSB: DUNVIEW.C F0128 lines 8528-8529. */
    CSB_F0128_D1C_ORDER = 15,      /* ReDMCSB: DUNVIEW.C F0128 lines 8530-8533. */
    CSB_F0128_D0L_ORDER = 16,      /* ReDMCSB: DUNVIEW.C F0128 lines 8534-8537. */
    CSB_F0128_D0R_ORDER = 17,      /* ReDMCSB: DUNVIEW.C F0128 lines 8538-8541. */
    CSB_F0127_ORDER = 18,          /* ReDMCSB: DUNVIEW.C F0128 line 8542. */
    CSB_F0127_OBJECT_PASS_LINE = 8294,
    CSB_D1_DEPTH = 1,
    CSB_D1L_LANE = -1,
    CSB_D1R_LANE = 1,
    CSB_ZONE_DOOR_D1L = 3780,      /* ReDMCSB: DEFS.H line 4258 M630_ZONE_DOOR_D1L. */
    CSB_ZONE_DOOR_D1R = 3800,      /* ReDMCSB: DEFS.H line 4260 M632_ZONE_DOOR_D1R. */
    CSB_ZONE_DOOR_FRAME_TOP_D1L = 732,
    CSB_ZONE_DOOR_FRAME_TOP_D1R = 734,
    CSB_ZONE_WALL_D1L = 713,       /* ReDMCSB: DEFS.H line 4053 C713_ZONE_WALL_D1L. */
    CSB_ZONE_WALL_D1R = 714,       /* ReDMCSB: DEFS.H line 4054 C714_ZONE_WALL_D1R. */
    CSB_DOOR_STATE_OPEN = 0,
    CSB_DOOR_STATE_PARTLY_ONE = 1,
    CSB_DOOR_STATE_PARTLY_TWO = 2,
    CSB_DOOR_STATE_PARTLY_THREE = 3,
    CSB_DOOR_STATE_CLOSED = 4,
    CSB_DOOR_STATE_DESTROYED = 5,
    CSB_C6_UNKNOWN = 6,            /* ReDMCSB: DEFS.H line 3508 C6_UNKNOWN. */
    CSB_SECOND_HALF_OFFSET = 3,
    CSB_MASK0X4000 = 0x4000,       /* ReDMCSB: DEFS.H line 3516 MASK0x4000... */
    CSB_C10_COLOR_FLESH = 10
};

static const char s_source_evidence[] =
    "PASS651 contract-only synthetic source-lock; no real-asset pixel parity "
    "and no game-data load. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor anchors the partly-open door dispatch: "
    "4311-4313 selects LeftHorizontal and RightHorizontal frame bitmaps, "
    "4317-4318 increments P2084_i_ZoneIndex by door state, 4320-4324 clips "
    "and blits the first horizontal half through zone + C6_UNKNOWN, and "
    "4325-4334 shifts the second half by 3 | MASK0x4000 before the C10 "
    "transparent blit. DUNVIEW.C:7391-7557 F0122 binds the D1L body and "
    "M630_ZONE_DOOR_D1L; DUNVIEW.C:7559-7725 F0123 binds the D1R body and "
    "M632_ZONE_DOOR_D1R. DUNVIEW.C:8524-8542 F0128 dispatches D1L, D1R, "
    "D1C, D0L, D0R, then F0127_DUNGEONVIEW_DrawSquareD0C; DUNVIEW.C:8294 "
    "anchors that F0127 object-pass follow-up. DEFS.H:2088,2605-2606,4047-4048 "
    "are retained as requested D2 baseline anchors, while DEFS.H:2600-2601, "
    "4053-4054,4258,4260 anchor the D1 side view squares, wall zones, and "
    "door zones. CSB-lineage Viewport.cpp:1903-1915 binds the CSB F1 "
    "door-facing dispatch around StdDrawDoor.";

#define D1_SPEC(side_value, route, square, func, order, lane, door_zone, top_zone, wall_zone, body_anchor, left_name, right_name) \
    { \
        side_value, route, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, square, func, \
        order, CSB_D1_DEPTH, lane, CSB_F0128_D1C_ORDER, CSB_F0128_D0L_ORDER, \
        CSB_F0128_D0R_ORDER, CSB_F0127_ORDER, CSB_F0127_OBJECT_PASS_LINE, \
        door_zone, top_zone, wall_zone, CSB_DOOR_STATE_OPEN, \
        CSB_DOOR_STATE_PARTLY_ONE, CSB_DOOR_STATE_PARTLY_TWO, \
        CSB_DOOR_STATE_PARTLY_THREE, CSB_DOOR_STATE_CLOSED, \
        CSB_DOOR_STATE_DESTROYED, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, \
        left_name, right_name, CSB_C6_UNKNOWN, CSB_C6_UNKNOWN, CSB_PRESENT, \
        CSB_PRESENT, CSB_PRESENT, CSB_C10_COLOR_FLESH, \
        CSB_SECOND_HALF_OFFSET, CSB_MASK0X4000, CSB_PRESENT, \
        CSB_C10_COLOR_FLESH, \
        "ReDMCSB DUNVIEW.C F0111 lines 4218-4337, 4311-4334 partly-open horizontal path", \
        body_anchor, \
        "ReDMCSB DUNVIEW.C F0128 lines 8524-8542 D1L/D1R dispatch and F0127 follow-up", \
        "ReDMCSB DUNVIEW.C F0127 line 8294 object-pass boundary", \
        "ReDMCSB DEFS.H lines 2088,2600-2601,2605-2606,4047-4048,4053-4054,4258,4260", \
        "CSB-lineage Viewport.cpp lines 1903-1915 StdDrawF1DoorFacing" \
    }

static const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 s_specs[] = {
    D1_SPEC(CSB_SIDE_D1L2,
            "D1L2 partly-open horizontal F0111 door via F0122 D1L body",
            CSB_VIEW_SQUARE_D1L,
            CSB_F0122,
            CSB_F0128_D1L_ORDER,
            CSB_D1L_LANE,
            CSB_ZONE_DOOR_D1L,
            CSB_ZONE_DOOR_FRAME_TOP_D1L,
            CSB_ZONE_WALL_D1L,
            "ReDMCSB DUNVIEW.C F0122 lines 7391-7557; door-front lines 7492-7508",
            "G0185_s_Graphic558_Frames_Door_D1L.LeftHorizontal[state-1]",
            "G0185_s_Graphic558_Frames_Door_D1L.RightHorizontal[state-1]"),
    D1_SPEC(CSB_SIDE_D1R2,
            "D1R2 partly-open horizontal F0111 door via F0123 D1R body",
            CSB_VIEW_SQUARE_D1R,
            CSB_F0123,
            CSB_F0128_D1R_ORDER,
            CSB_D1R_LANE,
            CSB_ZONE_DOOR_D1R,
            CSB_ZONE_DOOR_FRAME_TOP_D1R,
            CSB_ZONE_WALL_D1R,
            "ReDMCSB DUNVIEW.C F0123 lines 7559-7725; door-front lines 7660-7676",
            "G0187_s_Graphic558_Frames_Door_D1R.LeftHorizontal[state-1]",
            "G0187_s_Graphic558_Frames_Door_D1R.RightHorizontal[state-1]")
};

#undef D1_SPEC

size_t csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(int side)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
    if (door_state == spec->open_state) {
        return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_OPEN_PC34;
    }
    if (door_state == spec->closed_state) {
        return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_CLOSED_PC34;
    }
    if (door_state == spec->destroyed_state) {
        return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_DESTROYED_PC34;
    }
    if (door_state >= spec->partly_open_state_one &&
        door_state <= spec->partly_open_state_three) {
        return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    }
    return CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_INVALID_PC34;
}

const char *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int right_half)
{
    if (csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4308-4313 decrements state and then
     * selects LeftHorizontal[state-1] and RightHorizontal[state-1]. */
    return right_half ? spec->right_horizontal_frame_bitmap :
                        spec->left_horizontal_frame_bitmap;
}

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (!horizontal_door) return -1;
    if (csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    /* ReDMCSB: DUNVIEW.C F0111 lines 4317-4324. */
    return spec->door_zone_base + door_state + spec->first_half_dest_zone_offset;
}

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return -1;
    }
    if (!horizontal_door) return spec->door_zone_base + door_state;

    /* ReDMCSB: DUNVIEW.C F0111 lines 4325-4334. */
    return spec->door_zone_base + door_state +
           (spec->second_half_zone_offset | spec->second_half_zone_mask);
}

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *destination,
    int destination_width,
    int destination_height,
    int destination_stride,
    int *out_c10_skipped)
{
    int copied = 0;
    int skipped = 0;

    if (!spec || !source || !destination) return -1;
    if (source_width <= 0 || source_height <= 0) return -1;
    if (source_stride < source_width || destination_stride < destination_width) {
        return -1;
    }
    if (destination_width < source_width || destination_height < source_height) {
        return -1;
    }
    if (csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) !=
        CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34) {
        return 0;
    }

    /* ReDMCSB: DUNVIEW.C F0111 line 4334 uses C10_COLOR_FLESH transparency;
     * this helper is synthetic and only proves the C10 skip contract. */
    for (int y = 0; y < source_height; ++y) {
        for (int x = 0; x < source_width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->second_half_transparent_color) {
                ++skipped;
                continue;
            }
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    if (out_c10_skipped) *out_c10_skipped = skipped;
    return copied;
}

int csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_D1L2D1R2F0111PartlyOpenDoorProbePc34 *out_probe)
{
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_SIDE_D1L2);
    const CSB_V1_D1L2D1R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_SIDE_D1R2);
    const uint8_t source[6] = { 10, 1, 2, 10, 3, 4 };
    uint8_t destination[6] = { 0, 0, 0, 0, 0, 0 };
    int skipped = 0;
    int copied;

    if (!out_probe || !left || !right) return -1;
    copied = csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_synthetic_blit_pc34(
        left, CSB_DOOR_STATE_PARTLY_TWO, source, 3, 2, 3, destination, 3, 2, 3,
        &skipped);

    out_probe->route_count =
        (int)csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_spec_count_pc34();
    out_probe->dispatch_order_ok =
        left->f0128_dispatch_order == CSB_F0128_D1L_ORDER &&
        right->f0128_dispatch_order == CSB_F0128_D1R_ORDER &&
        left->f0128_dispatch_order < right->f0128_dispatch_order;
    out_probe->branch_state_ok =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_branch_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO) ==
        CSB_V1_D1L2_D1R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34;
    out_probe->frame_selection_ok =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, 0) != 0 &&
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_frame_bitmap_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, 1) != 0;
    out_probe->first_half_zone =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_first_half_zone_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->second_half_zone =
        csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_second_half_zone_pc34(
            left, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->f0128_followup_ok =
        left->f0128_d1c_followup_order == CSB_F0128_D1C_ORDER &&
        left->f0128_d0l_followup_order == CSB_F0128_D0L_ORDER &&
        left->f0128_d0r_followup_order == CSB_F0128_D0R_ORDER &&
        left->f0127_followup_order == CSB_F0127_ORDER &&
        left->f0127_object_pass_line == CSB_F0127_OBJECT_PASS_LINE;
    out_probe->copied_pixels = copied;
    out_probe->c10_skipped_pixels = skipped;
    out_probe->no_real_asset_pixel_parity = CSB_PRESENT;

    return copied == 4 && skipped == 2 ? 0 : -1;
}

const char *
csb_v1_viewport_d1l2_d1r2_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
