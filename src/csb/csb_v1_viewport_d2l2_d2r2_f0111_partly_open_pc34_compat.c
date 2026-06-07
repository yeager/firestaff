#include "csb_v1_viewport_d2l2_d2r2_f0111_partly_open_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_VIEW_SQUARE_D2L2 = 9,          /* ReDMCSB: DEFS.H:2605 C09_VIEW_SQUARE_D2L2. */
    CSB_VIEW_SQUARE_D2R2 = 10,         /* ReDMCSB: DEFS.H:2606 C10_VIEW_SQUARE_D2R2. */
    CSB_D2_DRAW_DEPTH = 2,             /* ReDMCSB: DUNVIEW.C:8503/8507 F0128. */
    CSB_D2L2_LATERAL = -2,             /* ReDMCSB: DUNVIEW.C:8503 F0128. */
    CSB_D2R2_LATERAL = 2,              /* ReDMCSB: DUNVIEW.C:8507 F0128. */
    CSB_D2L2_DRAW_ORDER = 8,           /* ReDMCSB: DUNVIEW.C:8503-8504 F0128 order. */
    CSB_D2R2_DRAW_ORDER = 9,           /* ReDMCSB: DUNVIEW.C:8507-8508 F0128 order. */
    CSB_C707_ZONE_WALL_D2L2 = 707,     /* ReDMCSB: DEFS.H:4047 C707_ZONE_WALL_D2L2. */
    CSB_C708_ZONE_WALL_D2R2 = 708,     /* ReDMCSB: DEFS.H:4048 C708_ZONE_WALL_D2R2. */
    CSB_C3700_ZONE_DOOR_D3L2 = 3700,   /* ReDMCSB: DEFS.H:4250 C3700_ZONE_DOOR_D3L2. */
    CSB_DOOR_STATE_OPEN = 0,           /* ReDMCSB: DEFS.H:1039 C0_DOOR_STATE_OPEN. */
    CSB_DOOR_STATE_CLOSED = 4,         /* ReDMCSB: DEFS.H:1043 C4_DOOR_STATE_CLOSED. */
    CSB_DOOR_STATE_DESTROYED = 5,      /* ReDMCSB: DEFS.H:1044 C5_DOOR_STATE_DESTROYED. */
    CSB_COORD_CLOSED_RECORD_TYPE = 1,  /* ReDMCSB: COORD.C:788 C3700 closed record. */
    CSB_COORD_PARENT_RECORD = 129,     /* ReDMCSB: COORD.C:788/1559 parent record. */
    CSB_COORD_CLIP_RECORD = 126,       /* ReDMCSB: COORD.C:1556/1559 clip record. */
    CSB_COORD_FRAME_X = 24,            /* ReDMCSB: COORD.C:1559 parent x. */
    CSB_COORD_FRAME_Y = 28,            /* ReDMCSB: COORD.C:1559 parent y. */
    CSB_NATIVE_BITMAP_WIDTH = 48,       /* ReDMCSB: COORD.C:1550 native width. */
    CSB_NATIVE_BITMAP_HEIGHT = 41,      /* ReDMCSB: COORD.C:1550 native height. */
    CSB_CLIPPED_WIDTH = 48,             /* ReDMCSB: COORD.C:1556 clip width. */
    CSB_CLIPPED_HEIGHT = 40,            /* ReDMCSB: COORD.C:1556 clip height. */
    CSB_C6_UNKNOWN = 6,                 /* ReDMCSB: DEFS.H:3508 C6_UNKNOWN. */
    CSB_FINAL_HALF_OFFSET = 3,          /* ReDMCSB: DUNVIEW.C:4325 final-half +3. */
    CSB_MASK_4000 = 0x4000,             /* ReDMCSB: DEFS.H:3516 MASK0x4000. */
    CSB_C10_COLOR_FLESH = 10,           /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_LINEAGE_REAR_ORDER = 0x0218,    /* CSB-lineage Viewport.cpp:1816. */
    CSB_LINEAGE_FRONT_ORDER = 0x0349    /* CSB-lineage Viewport.cpp:1820. */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset pixel parity and no CSB "
    "game-data load. ReDMCSB DUNVIEW.C:8503-8508 F0128 binds D2L2/D2R2 to "
    "depth 2 lateral -2/+2 before F0678/F0679. DUNVIEW.C:6837-6896 binds "
    "their wall/teleporter path to C707/C708 and returns wall cases before "
    "F0111, F0115, or F0107 back-wall ornament routing. DUNVIEW.C:4218-4337 "
    "F0111 skips C0 open doors, keeps closed/destroyed on the base zone, "
    "uses lines 4317-4325 for partly-open horizontal door math, blits the "
    "first half at P2084 + DoorState + C6_UNKNOWN, shifts the final half by "
    "3 | MASK0x4000, and blits with C10 at line 4334. COORD.C:788-797 "
    "provides the C3700 per-state zone records and COORD.C:1556-1559 "
    "provides the 48x40 clip record 126 through parent 129 at x=24,y=28. "
    "DEFS.H:2088 anchors C10_COLOR_FLESH, DEFS.H:2605-2606 anchors C09/C10 "
    "view squares, DEFS.H:3508/3516 anchors C6 and MASK0x4000, "
    "DEFS.H:4047-4048 anchors C707/C708, and DEFS.H:4250 anchors C3700 "
    "D3 door metadata that the D2L2/D2R2 dispatchers explicitly exclude. "
    "CSB-lineage Viewport.cpp:1813-1820 binds the F3L1 door-facing command "
    "stream to rear order 0x0218, frame blit, StdDrawDoor, and front order "
    "0x0349; this gate records that binding as lineage evidence only.";

static const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 s_specs[] = {
    {
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D2L2,
        CSB_D2L2_DRAW_ORDER,
        CSB_D2_DRAW_DEPTH,
        CSB_D2L2_LATERAL,
        CSB_C707_ZONE_WALL_D2L2,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_COORD_CLOSED_RECORD_TYPE,
        CSB_COORD_PARENT_RECORD,
        CSB_COORD_CLIP_RECORD,
        CSB_COORD_FRAME_X,
        CSB_COORD_FRAME_Y,
        CSB_NATIVE_BITMAP_WIDTH,
        CSB_NATIVE_BITMAP_HEIGHT,
        CSB_CLIPPED_WIDTH,
        CSB_CLIPPED_HEIGHT,
        CSB_DOOR_STATE_OPEN,
        1,
        2,
        CSB_DOOR_STATE_CLOSED,
        CSB_DOOR_STATE_DESTROYED,
        CSB_C6_UNKNOWN,
        CSB_FINAL_HALF_OFFSET,
        CSB_MASK_4000,
        CSB_C10_COLOR_FLESH,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_LINEAGE_REAR_ORDER,
        CSB_LINEAGE_FRONT_ORDER,
        "D2L2 F0111 partly-open door zone/clip contract",
        "ReDMCSB DUNVIEW.C:6837-6865 F0678 and 8503-8504 F0128",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111",
        "ReDMCSB COORD.C:788-797 and 1556-1559",
        "ReDMCSB DEFS.H:2088,2605,3508,3516,4047,4250",
        "CSB-lineage Viewport.cpp:1813-1820"
    },
    {
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D2R2,
        CSB_D2R2_DRAW_ORDER,
        CSB_D2_DRAW_DEPTH,
        CSB_D2R2_LATERAL,
        CSB_C708_ZONE_WALL_D2R2,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_ABSENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_C3700_ZONE_DOOR_D3L2,
        CSB_COORD_CLOSED_RECORD_TYPE,
        CSB_COORD_PARENT_RECORD,
        CSB_COORD_CLIP_RECORD,
        CSB_COORD_FRAME_X,
        CSB_COORD_FRAME_Y,
        CSB_NATIVE_BITMAP_WIDTH,
        CSB_NATIVE_BITMAP_HEIGHT,
        CSB_CLIPPED_WIDTH,
        CSB_CLIPPED_HEIGHT,
        CSB_DOOR_STATE_OPEN,
        1,
        2,
        CSB_DOOR_STATE_CLOSED,
        CSB_DOOR_STATE_DESTROYED,
        CSB_C6_UNKNOWN,
        CSB_FINAL_HALF_OFFSET,
        CSB_MASK_4000,
        CSB_C10_COLOR_FLESH,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_LINEAGE_REAR_ORDER,
        CSB_LINEAGE_FRONT_ORDER,
        "D2R2 F0111 partly-open door zone/clip contract",
        "ReDMCSB DUNVIEW.C:6868-6896 F0679 and 8507-8508 F0128",
        "ReDMCSB DUNVIEW.C:4218-4337 F0111",
        "ReDMCSB COORD.C:788-797 and 1556-1559",
        "ReDMCSB DEFS.H:2088,2606,3508,3516,4048,4250",
        "CSB-lineage Viewport.cpp:1813-1820"
    }
};

static const int s_c3700_record_types[10] = { 1, 4, 4, 4, 2, 2, 2, 1, 1, 1 };
static const int s_c3700_record_x[10] = { 0, 0, 0, 0, 6, 12, 18, 42, 36, 30 };
static const int s_c3700_record_y[10] = { 0, 10, 20, 30, 0, 0, 0, 0, 0, 0 };

size_t csb_v1_viewport_d2l2_d2r2_f0111_partly_open_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_f0111_partly_open_count_pc34()) {
        return NULL;
    }
    return &s_specs[index];
}

const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_for_square_pc34(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d2l2_d2r2_f0111_partly_open_count_pc34(); ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return NULL;
}

const CSB_V1_ViewportDoorPanelBlitSpec *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_context_panel_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C:4218-4337 F0111 panel math is shared by the
     * source-locked CSB viewport context; the D2 route gate consumes the
     * existing C3700 panel metadata without adding shared state. */
    return csb_v1_viewport_get_door_panel_blit_spec(0);
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_first_half_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (!spec || door_state < 0) return -1;
    if (door_state == spec->open_state) return -1;
    if (!horizontal_door) return -1;
    if (door_state == spec->closed_state || door_state == spec->destroyed_state) return -1;
    return spec->door_zone_base + door_state + spec->first_half_zone_offset;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_final_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    int zone;

    if (!spec || door_state < 0) return -1;
    if (door_state == spec->open_state) return -1;
    if (door_state == spec->closed_state || door_state == spec->destroyed_state) {
        return spec->door_zone_base;
    }
    zone = spec->door_zone_base + door_state;
    if (horizontal_door) {
        zone += spec->final_half_zone_offset;
        zone |= spec->final_half_mask;
    }
    return zone;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int zone,
    int *out_record_type,
    int *out_x,
    int *out_y)
{
    int base_zone;
    int index;

    if (!spec || !out_record_type || !out_x || !out_y) return -1;
    base_zone = zone & ~spec->final_half_mask;
    index = base_zone - spec->door_zone_base;
    if (index < 0 || index >= 10) return -1;

    *out_record_type = s_c3700_record_types[index];
    *out_x = s_c3700_record_x[index];
    *out_y = s_c3700_record_y[index];
    return 0;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_resolve_clip_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int zone,
    int *out_x,
    int *out_y)
{
    int record_type;
    int x;
    int y;

    if (!spec || !out_x || !out_y) return -1;
    if (csb_v1_viewport_d2l2_d2r2_f0111_partly_open_coord_for_zone_pc34(
            spec, zone, &record_type, &x, &y) != 0) {
        return -1;
    }
    (void)record_type;
    *out_x = spec->coord_frame_x + x;
    *out_y = spec->coord_frame_y + y;
    return 0;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2L2D2R2F0111PartlyOpenSpecPc34 *spec,
    int door_state,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!spec || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (width > spec->clipped_width || height > spec->clipped_height) return -1;
    if (source_stride < width || destination_stride < width) return -1;
    if (door_state == spec->open_state) return 0;

    /* ReDMCSB: DUNVIEW.C:4334 F0111 passes C10_COLOR_FLESH to the final
     * blit; COORD.C:1556-1559 constrains this synthetic gate to the 48x40
     * panel clip inherited through record 129. */
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

const char *csb_v1_viewport_d2l2_d2r2_f0111_partly_open_source_evidence_pc34(void)
{
    return s_source_evidence;
}
