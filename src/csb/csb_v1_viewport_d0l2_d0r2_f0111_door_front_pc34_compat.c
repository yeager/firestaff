#include "csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D3L = 12,
    CSB_VIEW_SQUARE_D3R = 13,
    CSB_DEPTH_D3 = 3,
    CSB_LATERAL_D3L = -1,
    CSB_LATERAL_D3R = 1,
    CSB_WALL_ZONE_D3L = 705,
    CSB_WALL_ZONE_D3R = 706,
    CSB_DOOR_ZONE_D3L = 3720,
    CSB_DOOR_ZONE_D3R = 3740,
    CSB_FLOOR_VIEW_D3L = 2,
    CSB_FLOOR_VIEW_D3R = 4,
    CSB_DOOR_FRONT_BITMAP_D3LCR = 693,
    CSB_DOOR_ORNAMENT_D3LCR = 0,
    CSB_C10_COLOR_FLESH = 10,
    CSB_LINEAGE_STD_ROOM_OBJECTS = 60006, /* CSB-lineage Viewport.cpp:379. */
    CSB_LINEAGE_DRAWORDER218 = 60279,    /* CSB-lineage Viewport.cpp:681/1906. */
    CSB_LINEAGE_DRAWORDER349 = 60280     /* CSB-lineage Viewport.cpp:682/1915. */
};

static const char s_source_evidence[] =
    "CSB V1 D0L2/D0R2 F0111 door-front source-lock; contract-only, "
    "asset-free, and no CSB game-data load. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor skips C0 open doors at 4248-4253, copies "
    "the native door bitmap at 4255-4262, overlays the door ornament at "
    "4262, applies optional animated flips at 4263-4287, adds the thieves "
    "eye mask at 4288-4294, draws closed/destroyed doors at 4297-4305, "
    "decrements partly-open state at 4307-4318, and performs the PC34 "
    "C10-transparent final blit at 4334. ReDMCSB DUNVIEW.C:6442-6460 "
    "and 6578-6602 are the D3L/D3R door-front callers mirrored by this "
    "CSB D0L2/D0R2 compatibility surface: F0108 floor ornament first, "
    "F0115 rear pass 0x0218/0x0128, F0111 front-door bitmap, then F0115 "
    "front pass 0x0349/0x0439. ReDMCSB DUNGEON.C:2466-2523 F0172 supplies "
    "M557/M556/M558/M550 square-aspect fields; DUNGEON.C:1769-1838 F0163 "
    "and 1840-1905 F0164 are thing-list mutation anchors that the draw "
    "contract must not call. ReDMCSB DEFS.H:2088 anchors C10_COLOR_FLESH; "
    "DEFS.H:2668-2675 anchors the rear/front door cell orders; DEFS.H:"
    "2789 anchors C0_VIEW_DOOR_ORNAMENT_D3LCR; DEFS.H:4045-4046 anchors "
    "C705/C706 wall zones and 4252-4254 anchors M624/M626 door zones. "
    "CSB-lineage Viewport.cpp:1903-1915 binds the F1 door-facing two-pass "
    "room-object shape with DrawOrder218 before StdDrawDoor and DrawOrder349 "
    "after it; Viewport.cpp:1930-1944 keeps F0L1/F0R1 door-facing return-only "
    "so this gate is the distinct D0L2/D0R2 compatibility surface; "
    "Viewport.cpp:1192-1209 is retained as the open-room contrast.";

static const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 s_specs[] = {
    {
        CSB_V1_D0L2_D0R2_F0111_SIDE_D0L2_PC34,
        "D0L2 compatibility surface over D3L door-front composition",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_DEPTH_D3,
        CSB_LATERAL_D3L,
        0,
        CSB_VIEW_SQUARE_D3L,
        CSB_WALL_ZONE_D3L,
        CSB_DOOR_ZONE_D3L,
        CSB_FLOOR_VIEW_D3L,
        0x0218u,
        0x0349u,
        CSB_DOOR_FRONT_BITMAP_D3LCR,
        CSB_DOOR_ORNAMENT_D3LCR,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        "ReDMCSB DUNVIEW.C:4218-4337 F0111; caller at 6442-6460",
        "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L; 4252 M624_ZONE_DOOR_D3L",
        "CSB-lineage Viewport.cpp:1903-1915; 1930-1944; 1192-1209",
        s_source_evidence
    },
    {
        CSB_V1_D0L2_D0R2_F0111_SIDE_D0R2_PC34,
        "D0R2 compatibility surface over D3R door-front composition",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_DEPTH_D3,
        CSB_LATERAL_D3R,
        1,
        CSB_VIEW_SQUARE_D3R,
        CSB_WALL_ZONE_D3R,
        CSB_DOOR_ZONE_D3R,
        CSB_FLOOR_VIEW_D3R,
        0x0128u,
        0x0439u,
        CSB_DOOR_FRONT_BITMAP_D3LCR,
        CSB_DOOR_ORNAMENT_D3LCR,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        "ReDMCSB DUNVIEW.C:4218-4337 F0111; caller at 6578-6602",
        "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; 2466-2523 F0172",
        "ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R; 4254 M626_ZONE_DOOR_D3R",
        "CSB-lineage Viewport.cpp:1903-1915; 1930-1944; 1192-1209",
        s_source_evidence
    }
};

size_t csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *
csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_decode_cell_pc34(
    unsigned int order,
    int ordinal)
{
    unsigned int shift;
    unsigned int cell;

    /* ReDMCSB: DUNVIEW.C F0115 lines 4547-4581 consumes low nibbles of
     * DEFS.H lines 2668-2675 cell-order constants until a zero terminator. */
    if (ordinal < 0 || ordinal > 3) return -1;
    shift = (unsigned int)ordinal * 4u;
    cell = (order >> shift) & 0x0fu;
    if (cell == 0u || cell == 8u || cell == 9u) return -1;
    return (int)cell - 1;
}

uint8_t csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB: DUNVIEW.C F0111 line 4334 passes DEFS.H line 2088
     * C10_COLOR_FLESH to the final PC34 door-front blit. */
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_compose_pixel_pc34(
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *spec,
    uint8_t base_pixel,
    uint8_t floor_pixel,
    uint8_t rear_pass_pixel,
    uint8_t door_pixel,
    uint8_t front_pass_pixel,
    CSB_V1_D0L2D0R2F0111DoorFrontTracePc34 *out_trace)
{
    CSB_V1_D0L2D0R2F0111DoorFrontTracePc34 trace;

    if (!spec || !out_trace) return -1;
    trace.ok = CSB_PRESENT;
    trace.f0108_calls = 1;
    trace.f0115_calls = 2;
    trace.f0111_calls = 1;
    trace.floor_transparent = floor_pixel == CSB_C10_COLOR_FLESH;
    trace.rear_transparent = rear_pass_pixel == CSB_C10_COLOR_FLESH;
    trace.door_transparent = door_pixel == CSB_C10_COLOR_FLESH;
    trace.front_transparent = front_pass_pixel == CSB_C10_COLOR_FLESH;
    trace.after_floor =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(base_pixel, floor_pixel);
    trace.after_rear_pass =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(trace.after_floor,
                                                              rear_pass_pixel);
    trace.after_door =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(trace.after_rear_pass,
                                                              door_pixel);
    trace.after_front_pass =
        csb_v1_viewport_d0l2_d0r2_f0111_door_front_blend_pc34(trace.after_door,
                                                              front_pass_pixel);
    *out_trace = trace;
    return 0;
}

int csb_v1_viewport_d0l2_d0r2_f0111_door_front_is_draw_mutating_pc34(
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *spec)
{
    if (!spec) return -1;
    /* ReDMCSB: DUNGEON.C F0163/F0164 mutate thing lists, while F0111 draws
     * from the square aspect supplied by F0172 and does not link or unlink. */
    return !(spec->f0163_not_called_by_draw && spec->f0164_not_called_by_draw);
}

const char *csb_v1_viewport_d0l2_d0r2_f0111_door_front_source_evidence_pc34(void)
{
    return s_source_evidence;
}
