#include "csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat.h"

enum {
    CSB_ROUTE_PRESENT = 1,
    CSB_ROUTE_ABSENT = 0,
    CSB_VIEW_SQUARE_D1L = 4,         /* ReDMCSB: DEFS.H:2600 M607_VIEW_SQUARE_D1L. */
    CSB_VIEW_SQUARE_D1R = 5,         /* ReDMCSB: DEFS.H:2601 M608_VIEW_SQUARE_D1R. */
    CSB_VIEW_DEPTH_D1 = 1,           /* ReDMCSB: DUNVIEW.C:372 G2027[4/5]. */
    CSB_VIEW_LANE_LEFT = -1,         /* ReDMCSB: DUNVIEW.C:371 G2026[4]. */
    CSB_VIEW_LANE_RIGHT = 1,         /* ReDMCSB: DUNVIEW.C:371 G2026[5]. */
    CSB_D1L_PASS1 = 0x0028,          /* ReDMCSB: DUNVIEW.C:7494. */
    CSB_D1L_PASS2 = 0x0039,          /* ReDMCSB: DUNVIEW.C:7508. */
    CSB_D1R_PASS1 = 0x0018,          /* ReDMCSB: DUNVIEW.C:7662. */
    CSB_D1R_PASS2 = 0x0049,          /* ReDMCSB: DUNVIEW.C:7676. */
    CSB_C732_D1L_TOP_TRACK = 732,    /* ReDMCSB: DEFS.H:4091 C732_ZONE_DOOR_FRAME_TOP_D1L. */
    CSB_C734_D1R_TOP_TRACK = 734,    /* ReDMCSB: DEFS.H:4093 C734_ZONE_DOOR_FRAME_TOP_D1R. */
    CSB_M630_D1L_DOOR_ZONE = 3780,   /* ReDMCSB: DEFS.H:4258 M630_ZONE_DOOR_D1L. */
    CSB_M632_D1R_DOOR_ZONE = 3800,   /* ReDMCSB: DEFS.H:4260 M632_ZONE_DOOR_D1R. */
    CSB_C713_D1L_FIELD_ZONE = 713,   /* ReDMCSB: DEFS.H:4053 C713_ZONE_WALL_D1L. */
    CSB_C714_D1R_FIELD_ZONE = 714,   /* ReDMCSB: DEFS.H:4054 C714_ZONE_WALL_D1R. */
    CSB_C2_D1_DOOR_ORNAMENT = 2,     /* ReDMCSB: DEFS.H:2791 C2_VIEW_DOOR_ORNAMENT_D1LCR. */
    CSB_DOOR_GRAPHICS_F1 = 0,        /* CSB-lineage Viewport.cpp:2568 StdDoorGraphicsF1. */
    CSB_DOOR_STATE_OPEN = 0,         /* ReDMCSB: DEFS.H:1039 C0_DOOR_STATE_OPEN. */
    CSB_DOOR_STATE_CLOSED = 4,       /* ReDMCSB: DEFS.H:1043 C4_DOOR_STATE_CLOSED. */
    CSB_DOOR_STATE_DESTROYED = 5,    /* ReDMCSB: DEFS.H:1044 C5_DOOR_STATE_DESTROYED. */
    CSB_C15_DESTROYED_MASK = 15,     /* ReDMCSB: DEFS.H:2466 C15_DOOR_ORNAMENT_DESTROYED_MASK. */
    CSB_C6_UNKNOWN = 6,              /* ReDMCSB: DEFS.H:3508 C6_UNKNOWN. */
    CSB_MASK_4000 = 0x4000,          /* ReDMCSB: DEFS.H:3516 horizontal half zone shift. */
    CSB_C10_COLOR_FLESH = 10         /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
};

static const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatEvidence s_evidence = {
    "Source-locked contract gate only; no real-asset bitmap parity and no "
    "CSB game-data load.",
    "ReDMCSB DUNVIEW.C:7492-7508,7520-7536 F0122_DUNGEONVIEW_DrawSquareD1L",
    "ReDMCSB DUNVIEW.C:7660-7676,7688-7704 F0123_DUNGEONVIEW_DrawSquareD1R",
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
    "ReDMCSB DEFS.H:2599-2601,2791,4258-4260,2078-2088",
    "ReDMCSB COORD.C:780-877 door-zone range,1548-1567 door records",
    "ReDMCSB DUNVIEW.C:7520-7536/7688-7704 corridor open path; "
    "CSB-lineage Viewport.cpp:1892-1900,1919-1927 F1L1/F1R1 door facing",
    "ReDMCSB DUNVIEW.C:7542-7555/7709-7722 teleporter field after D1 pass",
    "ReDMCSB DUNVIEW.C:7502-7506/7670-7674 frame top plus F0111; "
    "CSB-lineage Viewport.cpp:1896-1898,1923-1925 frame-blt/frame-rect binding"
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity and no "
    "CSB game-data load. ReDMCSB DUNVIEW.C:7492-7508 locks the D1L "
    "door-front route: F0108 floor ornament, F0115 pass1 with order 0x0028, "
    "D1 top-track frame, F0111 with M630_ZONE_DOOR_D1L, then F0115 pass2 "
    "order 0x0039. ReDMCSB DUNVIEW.C:7660-7676 locks the mirrored D1R "
    "door-front route with order 0x0018, M632_ZONE_DOOR_D1R, and order "
    "0x0049. DUNVIEW.C:4218-4337 F0111 skips open doors, applies C15 "
    "destroyed masks, shifts partial door zones, and blits with C10. "
    "DEFS.H:2599-2601 maps D1C/D1L/D1R view squares, DEFS.H:2791 anchors "
    "C2_VIEW_DOOR_ORNAMENT_D1LCR, DEFS.H:4258-4260 anchors D1L/D1R door "
    "zones, and DEFS.H:2088 anchors C10_COLOR_FLESH. COORD.C:780-877 and "
    "1548-1567 provide the PC34 door-zone/door-record layout metadata. "
    "CSB-lineage Viewport.cpp:1892-1900 and 1919-1927 bind F1L1/F1R1 "
    "door facing to room-object rear pass, frame blt/rect, StdDrawDoor, "
    "and room-object front pass.";

static const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant s_invariants[] = {
    {
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D1L,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_LEFT,
        CSB_ROUTE_PRESENT,
        CSB_D1L_PASS1,
        CSB_ROUTE_PRESENT,
        CSB_D1L_PASS2,
        CSB_ROUTE_PRESENT,
        CSB_C732_D1L_TOP_TRACK,
        CSB_M630_D1L_DOOR_ZONE,
        CSB_C2_D1_DOOR_ORNAMENT,
        CSB_DOOR_GRAPHICS_F1,
        CSB_C713_D1L_FIELD_ZONE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_C15_DESTROYED_MASK,
        CSB_ROUTE_PRESENT,
        CSB_MASK_4000,
        CSB_C10_COLOR_FLESH,
        "D1L F0111 door-front route",
        "ReDMCSB DUNVIEW.C:7492-7508,7520-7536 F0122 D1L"
    },
    {
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_VIEW_SQUARE_D1R,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_RIGHT,
        CSB_ROUTE_PRESENT,
        CSB_D1R_PASS1,
        CSB_ROUTE_PRESENT,
        CSB_D1R_PASS2,
        CSB_ROUTE_PRESENT,
        CSB_C734_D1R_TOP_TRACK,
        CSB_M632_D1R_DOOR_ZONE,
        CSB_C2_D1_DOOR_ORNAMENT,
        CSB_DOOR_GRAPHICS_F1,
        CSB_C714_D1R_FIELD_ZONE,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_ROUTE_PRESENT,
        CSB_C15_DESTROYED_MASK,
        CSB_ROUTE_PRESENT,
        CSB_MASK_4000,
        CSB_C10_COLOR_FLESH,
        "D1R F0111 door-front route",
        "ReDMCSB DUNVIEW.C:7660-7676,7688-7704 F0123 D1R"
    }
};

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_count(void)
{
    return sizeof(s_invariants) / sizeof(s_invariants[0]);
}

const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_at(size_t index)
{
    if (index >= csb_v1_viewport_d1l_d1r_f0111_door_pc34_count()) return NULL;
    return &s_invariants[index];
}

const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d1l_d1r_f0111_door_pc34_count(); ++i) {
        if (s_invariants[i].view_square == view_square) return &s_invariants[i];
    }
    return NULL;
}

int csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    int door_state)
{
    if (!invariant) return -1;
    if (door_state == CSB_DOOR_STATE_OPEN) return -1;
    if (door_state < 0 || door_state > CSB_DOOR_STATE_DESTROYED) return -1;
    if (door_state == CSB_DOOR_STATE_CLOSED ||
        door_state == CSB_DOOR_STATE_DESTROYED) {
        return invariant->door_zone_base;
    }
    return invariant->door_zone_base + door_state;
}

int csb_v1_viewport_d1l_d1r_f0111_door_horizontal_half_zone_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    int door_state,
    int right_half)
{
    const int shifted = csb_v1_viewport_d1l_d1r_f0111_door_zone_for_state_pc34(
        invariant, door_state);
    if (shifted < 0 || door_state >= CSB_DOOR_STATE_CLOSED) return -1;
    if (!right_half) return shifted + CSB_C6_UNKNOWN;
    return shifted + (3 | invariant->horizontal_second_half_mask);
}

int csb_v1_viewport_d1l_d1r_f0111_door_apply_c10_blit_pc34(
    const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatInvariant *invariant,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!invariant || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;

    /* ReDMCSB: DUNVIEW.C:4334 F0111 blits through F0791 with
     * C10_COLOR_FLESH; this helper keeps the gate synthetic only. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)invariant->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const CSB_V1_ViewportD1LD1RF0111DoorPc34CompatEvidence *
csb_v1_viewport_d1l_d1r_f0111_door_evidence_pc34(void)
{
    return &s_evidence;
}

const char *csb_v1_viewport_d1l_d1r_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
