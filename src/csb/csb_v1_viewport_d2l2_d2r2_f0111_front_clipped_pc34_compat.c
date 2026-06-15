#include "csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_VIEW_SQUARE_D2L2 = 9,          /* ReDMCSB: DEFS.H:2605 C09_VIEW_SQUARE_D2L2. */
    CSB_VIEW_SQUARE_D2R2 = 10,         /* ReDMCSB: DEFS.H:2606 C10_VIEW_SQUARE_D2R2. */
    CSB_D2_DRAW_DEPTH = 2,             /* ReDMCSB: DUNVIEW.C:8503/8507 F0128. */
    CSB_D2L2_LATERAL = -2,             /* ReDMCSB: DUNVIEW.C:8503 F0128. */
    CSB_D2R2_LATERAL = 2,              /* ReDMCSB: DUNVIEW.C:8507 F0128. */
    CSB_C707_ZONE_WALL_D2L2 = 707,     /* ReDMCSB: DEFS.H:4047 C707_ZONE_WALL_D2L2. */
    CSB_C708_ZONE_WALL_D2R2 = 708,     /* ReDMCSB: DEFS.H:4048 C708_ZONE_WALL_D2R2. */
    CSB_C3700_ZONE_DOOR_D3L2 = 3700,   /* ReDMCSB: DEFS.H:4250 C3700_ZONE_DOOR_D3L2. */
    CSB_DOOR_STATE_OPEN = 0,           /* ReDMCSB: DEFS.H:1039 C0_DOOR_STATE_OPEN. */
    CSB_DOOR_STATE_CLOSED = 4,         /* ReDMCSB: DEFS.H:1043 C4_DOOR_STATE_CLOSED. */
    CSB_DOOR_STATE_DESTROYED = 5,      /* ReDMCSB: DEFS.H:1044 C5_DOOR_STATE_DESTROYED. */
    CSB_C6_UNKNOWN = 6,                /* ReDMCSB: DEFS.H:3508 C6_UNKNOWN. */
    CSB_FINAL_HALF_OFFSET = 3,         /* ReDMCSB: DUNVIEW.C:4325 final-half +3. */
    CSB_C4000_HALF_ZONE_SHIFT = 0x4000,/* ReDMCSB: DEFS.H:3516 MASK0x4000. */
    CSB_C15_DESTROYED_MASK = 15,       /* ReDMCSB: DEFS.H:2466 C15_DOOR_ORNAMENT_DESTROYED_MASK. */
    CSB_C2_VIEW_ORNAMENT_D1LCR = 2,    /* ReDMCSB: DEFS.H:2791 C2_VIEW_DOOR_ORNAMENT_D1LCR. */
    CSB_C10_COLOR_FLESH = 10,          /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_NATIVE_BITMAP_WIDTH = 48,      /* ReDMCSB: COORD.C:1550 native width. */
    CSB_NATIVE_BITMAP_HEIGHT = 41,     /* ReDMCSB: COORD.C:1550 native height. */
    CSB_FRAME_CLIP_WIDTH = 48,         /* ReDMCSB: COORD.C:1556 clip width. */
    CSB_FRAME_CLIP_HEIGHT = 40,        /* ReDMCSB: COORD.C:1556 clip height. */
    CSB_HALF_CLIP_WIDTH = 24,          /* ReDMCSB: DUNVIEW.C:4320 M100(...) >> 1. */
    CSB_FRAME_X = 24,                  /* ReDMCSB: COORD.C:1559 parent x. */
    CSB_FRAME_Y = 28,                  /* ReDMCSB: COORD.C:1559 parent y. */
    CSB_D2L2_PASS1_ORDER = 0x0028,     /* CSB-lineage Viewport.cpp:1895,2616. */
    CSB_D2L2_PASS2_ORDER = 0x0039,     /* CSB-lineage Viewport.cpp:1899,2618. */
    CSB_D2R2_PASS1_ORDER = 0x0018,     /* CSB-lineage Viewport.cpp:1922,2614. */
    CSB_D2R2_PASS2_ORDER = 0x0049      /* CSB-lineage Viewport.cpp:1926,2620. */
};

static const char s_source_evidence[] =
    "Source-locked front-clipped CSB V1 D2L2/D2R2 F0111 contract gate; "
    "no real-asset runtime draw. ReDMCSB DUNVIEW.C:3082-3093 F0102 blits "
    "the temporary door bitmap through frame byte width, source x/y, and "
    "C10 transparency; DUNVIEW.C:3096-3108 F0103 flips frame bitmaps "
    "horizontally before the same C10 blit. DUNVIEW.C:4218-4337 F0111 "
    "skips C0 open, selects C4 closed, applies C15 destroyed ornament "
    "mask for C5 at 4301-4304, and lines 4317-4325 split horizontal doors "
    "with a half-width source shift and 3 | MASK0x4000. DEFS.H:2466 "
    "anchors C15_DOOR_ORNAMENT_DESTROYED_MASK, DEFS.H:2791 anchors "
    "C2_VIEW_DOOR_ORNAMENT_D1LCR, and DEFS.H:3516 anchors the C4000 "
    "horizontal half-zone shift. The D2L2/D2R2 front-clipped side contract "
    "keeps the right half of D2L2 and the left half of D2R2. CSB-lineage "
    "Viewport.cpp:1853-1862 and 1881-1888 route F2L1/F2R1 door-facing "
    "frames through StdDrawDoor, while Viewport.cpp:1895/1899/1922/1926 "
    "and 2614-2620 anchor the neighboring pass constants 0x28/0x39 and "
    "0x18/0x49.";

static const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 s_specs[] = {
    {
        CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2L2,
        CSB_VIEW_SQUARE_D2L2,
        CSB_D2_DRAW_DEPTH,
        CSB_D2L2_LATERAL,
        CSB_C707_ZONE_WALL_D2L2,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_ABSENT,
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_C2_VIEW_ORNAMENT_D1LCR,
        CSB_C15_DESTROYED_MASK,
        CSB_C10_COLOR_FLESH,
        CSB_DOOR_STATE_OPEN,
        CSB_DOOR_STATE_CLOSED,
        CSB_DOOR_STATE_DESTROYED,
        CSB_C6_UNKNOWN,
        CSB_FINAL_HALF_OFFSET,
        CSB_C4000_HALF_ZONE_SHIFT,
        CSB_NATIVE_BITMAP_WIDTH,
        CSB_NATIVE_BITMAP_HEIGHT,
        CSB_FRAME_CLIP_WIDTH,
        CSB_FRAME_CLIP_HEIGHT,
        CSB_HALF_CLIP_WIDTH,
        CSB_FRAME_X,
        CSB_FRAME_Y,
        CSB_HALF_CLIP_WIDTH,
        CSB_HALF_CLIP_WIDTH,
        0,
        CSB_D2L2_PASS1_ORDER,
        CSB_D2L2_PASS2_ORDER,
        CSB_ROUTE_PRESENT,
        "D2L2 F0111 front-clipped right-half door panel",
        "ReDMCSB DUNVIEW.C:3082-3093 F0102",
        "ReDMCSB DUNVIEW.C:3096-3108 F0103",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111",
        "ReDMCSB DEFS.H:2466,2791,3516",
        "CSB-lineage Viewport.cpp:1853-1862,1895,1899,2616,2618"
    },
    {
        CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_ROUTE_D2R2,
        CSB_VIEW_SQUARE_D2R2,
        CSB_D2_DRAW_DEPTH,
        CSB_D2R2_LATERAL,
        CSB_C708_ZONE_WALL_D2R2,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_C2_VIEW_ORNAMENT_D1LCR,
        CSB_C15_DESTROYED_MASK,
        CSB_C10_COLOR_FLESH,
        CSB_DOOR_STATE_OPEN,
        CSB_DOOR_STATE_CLOSED,
        CSB_DOOR_STATE_DESTROYED,
        CSB_C6_UNKNOWN,
        CSB_FINAL_HALF_OFFSET,
        CSB_C4000_HALF_ZONE_SHIFT,
        CSB_NATIVE_BITMAP_WIDTH,
        CSB_NATIVE_BITMAP_HEIGHT,
        CSB_FRAME_CLIP_WIDTH,
        CSB_FRAME_CLIP_HEIGHT,
        CSB_HALF_CLIP_WIDTH,
        CSB_FRAME_X,
        CSB_FRAME_Y,
        0,
        CSB_HALF_CLIP_WIDTH,
        0,
        CSB_D2R2_PASS1_ORDER,
        CSB_D2R2_PASS2_ORDER,
        CSB_ROUTE_PRESENT,
        "D2R2 F0111 front-clipped left-half door panel",
        "ReDMCSB DUNVIEW.C:3082-3093 F0102",
        "ReDMCSB DUNVIEW.C:3096-3108 F0103",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111",
        "ReDMCSB DEFS.H:2466,2791,3516",
        "CSB-lineage Viewport.cpp:1881-1888,1922,1926,2614,2620"
    }
};

size_t csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_for_route_pc34(
    CSB_V1_D2L2D2R2F0111FrontClippedRoutePc34 route)
{
    for (size_t i = 0; i < csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_count_pc34(); ++i) {
        if (s_specs[i].route == route) return &s_specs[i];
    }
    return NULL;
}

int csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_trace_pc34(
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *spec,
    int door_state,
    int horizontal_door,
    CSB_V1_D2L2D2R2F0111FrontClippedTracePc34 *out_trace)
{
    CSB_V1_D2L2D2R2F0111FrontClippedTracePc34 trace;

    if (!spec || !out_trace || door_state < 0) return -1;

    trace.ok = CSB_ROUTE_PRESENT;
    trace.branch = CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_NONE;
    trace.door_drawn = CSB_ROUTE_ABSENT;
    trace.pass_count = 0;
    trace.ornament_view = spec->door_ornament_view;
    trace.destroyed_mask_applied = CSB_ROUTE_ABSENT;
    trace.selected_bitmap_state = -1;
    trace.uses_closed_or_destroyed_frame = CSB_ROUTE_ABSENT;
    trace.first_half_zone = -1;
    trace.final_zone = -1;
    trace.final_zone_without_shift_mask = -1;
    trace.c4000_shift_applied = CSB_ROUTE_ABSENT;
    trace.half_zone_shift_x = 0;
    trace.source_x = spec->source_half_x;
    trace.source_width = spec->half_clip_width;
    trace.source_height = spec->frame_clip_height;
    trace.destination_frame_x = spec->frame_x;
    trace.destination_frame_y = spec->frame_y;
    trace.pass1_order = spec->pass1_order;
    trace.pass2_order = spec->pass2_order;
    trace.transparent_color = spec->transparent_color;
    trace.source_lock_evidence = s_source_evidence;

    if (door_state == spec->open_state) {
        *out_trace = trace;
        return 0;
    }

    trace.door_drawn = CSB_ROUTE_PRESENT;
    trace.selected_bitmap_state = door_state;

    if (door_state == spec->closed_state) {
        trace.branch = CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_CLOSED;
        trace.pass_count = 1;
        trace.uses_closed_or_destroyed_frame = CSB_ROUTE_PRESENT;
        trace.final_zone = spec->door_zone_base;
        trace.final_zone_without_shift_mask = spec->door_zone_base;
    } else if (door_state == spec->destroyed_state) {
        trace.branch = CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_DESTROYED;
        trace.pass_count = 1;
        trace.destroyed_mask_applied = CSB_ROUTE_PRESENT;
        trace.uses_closed_or_destroyed_frame = CSB_ROUTE_PRESENT;
        trace.final_zone = spec->door_zone_base;
        trace.final_zone_without_shift_mask = spec->door_zone_base;
    } else if (horizontal_door) {
        trace.branch =
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_HORIZONTAL;
        trace.pass_count = 2;
        trace.first_half_zone =
            spec->door_zone_base + door_state + spec->horizontal_first_half_offset;
        trace.final_zone_without_shift_mask =
            spec->door_zone_base + door_state + spec->horizontal_final_half_offset;
        trace.final_zone =
            trace.final_zone_without_shift_mask | spec->horizontal_half_zone_shift_mask;
        trace.c4000_shift_applied = CSB_ROUTE_PRESENT;
        trace.half_zone_shift_x = spec->zone_shift_x;
    } else {
        trace.branch =
            CSB_V1_D2L2_D2R2_F0111_FRONT_CLIPPED_BRANCH_PARTLY_VERTICAL;
        trace.pass_count = 1;
        trace.final_zone = spec->door_zone_base + door_state;
        trace.final_zone_without_shift_mask = trace.final_zone;
    }

    *out_trace = trace;
    return 0;
}

int csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_half_blit_pc34(
    const CSB_V1_D2L2D2R2F0111FrontClippedSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int destination_x,
    int height)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (door_state == spec->open_state) return 0;
    if (height <= 0 || height > spec->frame_clip_height) return -1;
    if (source_stride < spec->native_bitmap_width) return -1;
    if (destination_x < 0 || destination_stride < destination_x + spec->half_clip_width) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C:3082-3093 F0102 applies C10 transparency while
     * DUNVIEW.C:4317-4325 F0111 has already selected the clipped half-zone. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < spec->half_clip_width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + spec->source_half_x + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + destination_x + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_d2l2_d2r2_f0111_front_clipped_source_evidence_pc34(void)
{
    return s_source_evidence;
}
