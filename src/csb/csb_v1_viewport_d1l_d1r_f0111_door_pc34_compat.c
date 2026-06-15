#include "csb/csb_v1_viewport_d1l_d1r_f0111_door_pc34_compat.h"

enum {
    CSB_D1L_VIEW_SQUARE = 4,
    CSB_D1R_VIEW_SQUARE = 5,
    CSB_D1_DEPTH = 1,
    CSB_LANE_LEFT = -1,
    CSB_LANE_RIGHT = 1,
    CSB_D1L_FLOOR_VIEW = 594,
    CSB_D1R_FLOOR_VIEW = 596,
    CSB_D1L_FIELD_ZONE = 713,
    CSB_D1R_FIELD_ZONE = 714,
    CSB_D1L_DOOR_ZONE = 3780,
    CSB_D1R_DOOR_ZONE = 3800,
    CSB_D1L_TOP_TRACK_ZONE = 732,
    CSB_D1R_TOP_TRACK_ZONE = 734,
    CSB_D1L_REAR_ORDER = 0x0028,
    CSB_D1R_REAR_ORDER = 0x0018,
    CSB_D1L_FRONT_ORDER = 0x0039,
    CSB_D1R_FRONT_ORDER = 0x0049,
    CSB_D1L_CORRIDOR_ORDER = 0x0032,
    CSB_D1R_CORRIDOR_ORDER = 0x0041,
    CSB_DOOR_STATE_OPEN = 0,
    CSB_DOOR_STATE_CLOSED = 4,
    CSB_DOOR_STATE_DESTROYED = 5,
    CSB_C2_VIEW_DOOR_ORNAMENT_D1LCR = 2,
    CSB_STD_DOOR_GRAPHICS_F1 = 1,
    CSB_STD_DOOR_RECTS_F1L1 = 11,
    CSB_STD_DOOR_RECTS_F1R1 = 12,
    CSB_STD_TOP_TRACK_RECT_F1L1 = 21,
    CSB_STD_TOP_TRACK_RECT_F1R1 = 22,
    CSB_F1L1_DOOR_RECORD_INDEX = 31,
    CSB_F1R1_DOOR_RECORD_INDEX = 32,
    CSB_F1L1_DOOR_STATE = 41,
    CSB_F1R1_DOOR_STATE = 42,
    CSB_C6_UNKNOWN = 6,
    CSB_MASK_4000_SHIFT = 0x4000,
    CSB_C10_COLOR_FLESH = 10,
    CSB_RENDER_WIDTH = 16,
    CSB_RENDER_HEIGHT = 12
};

static const CSB_V1_ViewportD1LD1RF0111Route s_routes[] = {
    {
        CSB_D1L_VIEW_SQUARE,
        CSB_D1_DEPTH,
        CSB_LANE_LEFT,
        1,
        -1,
        CSB_D1L_FLOOR_VIEW,
        CSB_D1L_FIELD_ZONE,
        CSB_D1L_DOOR_ZONE,
        CSB_D1L_TOP_TRACK_ZONE,
        CSB_D1L_REAR_ORDER,
        CSB_D1L_FRONT_ORDER,
        CSB_D1L_CORRIDOR_ORDER,
        8525,
        7492,
        7506,
        1892,
        0x0028,
        CSB_STD_TOP_TRACK_RECT_F1L1,
        CSB_F1L1_DOOR_RECORD_INDEX,
        CSB_F1L1_DOOR_STATE,
        CSB_STD_DOOR_GRAPHICS_F1,
        CSB_STD_DOOR_RECTS_F1L1,
        0x0039,
        "D1L F0111 door front",
        "ReDMCSB DUNVIEW.C:7492-7508 F0122_DUNGEONVIEW_DrawSquareD1L",
        "CSB-lineage Viewport.cpp:1892-1900 StdDrawF1L1DoorFacing"
    },
    {
        CSB_D1R_VIEW_SQUARE,
        CSB_D1_DEPTH,
        CSB_LANE_RIGHT,
        1,
        1,
        CSB_D1R_FLOOR_VIEW,
        CSB_D1R_FIELD_ZONE,
        CSB_D1R_DOOR_ZONE,
        CSB_D1R_TOP_TRACK_ZONE,
        CSB_D1R_REAR_ORDER,
        CSB_D1R_FRONT_ORDER,
        CSB_D1R_CORRIDOR_ORDER,
        8529,
        7660,
        7674,
        1919,
        0x0018,
        CSB_STD_TOP_TRACK_RECT_F1R1,
        CSB_F1R1_DOOR_RECORD_INDEX,
        CSB_F1R1_DOOR_STATE,
        CSB_STD_DOOR_GRAPHICS_F1,
        CSB_STD_DOOR_RECTS_F1R1,
        0x0049,
        "D1R F0111 door front",
        "ReDMCSB DUNVIEW.C:7660-7676 F0123_DUNGEONVIEW_DrawSquareD1R",
        "CSB-lineage Viewport.cpp:1919-1927 StdDrawF1R1DoorFacing"
    }
};

static const CSB_V1_ViewportD1LD1RF0111Step s_steps[][5] = {
    {
        { CSB_V1_D1LR_STEP_FLOOR_ORNAMENT, 0, -1, CSB_D1L_VIEW_SQUARE,
          CSB_D1L_FLOOR_VIEW, -1, -1, CSB_C10_COLOR_FLESH, 7493,
          "F0108 floor ornament" },
        { CSB_V1_D1LR_STEP_REAR_OBJECTS, CSB_D1L_REAR_ORDER, -1,
          CSB_D1L_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7494,
          "F0115 rear objects/creatures/projectiles/explosions" },
        { CSB_V1_D1LR_STEP_TOP_TRACK, 0, CSB_D1L_TOP_TRACK_ZONE,
          CSB_D1L_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7503,
          "F0104 door-frame top track" },
        { CSB_V1_D1LR_STEP_F0111_DOOR, 0, CSB_D1L_DOOR_ZONE,
          CSB_D1L_VIEW_SQUARE, -1, CSB_STD_DOOR_GRAPHICS_F1,
          CSB_STD_DOOR_RECTS_F1L1, CSB_C10_COLOR_FLESH, 7506,
          "F0111 door front" },
        { CSB_V1_D1LR_STEP_FRONT_OBJECTS, CSB_D1L_FRONT_ORDER, -1,
          CSB_D1L_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7536,
          "F0115 front objects/creatures/projectiles/explosions" }
    },
    {
        { CSB_V1_D1LR_STEP_FLOOR_ORNAMENT, 0, -1, CSB_D1R_VIEW_SQUARE,
          CSB_D1R_FLOOR_VIEW, -1, -1, CSB_C10_COLOR_FLESH, 7661,
          "F0108 floor ornament" },
        { CSB_V1_D1LR_STEP_REAR_OBJECTS, CSB_D1R_REAR_ORDER, -1,
          CSB_D1R_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7662,
          "F0115 rear objects/creatures/projectiles/explosions" },
        { CSB_V1_D1LR_STEP_TOP_TRACK, 0, CSB_D1R_TOP_TRACK_ZONE,
          CSB_D1R_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7671,
          "F0104 door-frame top track" },
        { CSB_V1_D1LR_STEP_F0111_DOOR, 0, CSB_D1R_DOOR_ZONE,
          CSB_D1R_VIEW_SQUARE, -1, CSB_STD_DOOR_GRAPHICS_F1,
          CSB_STD_DOOR_RECTS_F1R1, CSB_C10_COLOR_FLESH, 7674,
          "F0111 door front" },
        { CSB_V1_D1LR_STEP_FRONT_OBJECTS, CSB_D1R_FRONT_ORDER, -1,
          CSB_D1R_VIEW_SQUARE, -1, -1, -1, CSB_C10_COLOR_FLESH, 7704,
          "F0115 front objects/creatures/projectiles/explosions" }
    }
};

static const CSB_V1_ViewportD1LD1RF0111Evidence s_evidence = {
    "CSB V1 source-lock contract only; no real-asset runtime regression.",
    "ReDMCSB DUNVIEW.C:4218-4337 F0111: non-open guard, state decrement "
    "for frame selection, LeftHorizontal/RightHorizontal split, zone + state, "
    "MASK 0x4000 half-door shift, C10 transparent F0791 blit.",
    "ReDMCSB DUNVIEW.C:8318-8542 F0128 dispatch: D1L line 8525 and D1R "
    "line 8529 after relative move (1,-1)/(1,1).",
    "ReDMCSB DUNVIEW.C:3113-3156 F0104, 3185-3225 F0105, 3502-3590 "
    "F0107, 3940-4011 F0108 wall/floor/ornament callers.",
    "ReDMCSB DUNGEON.C:1769-1838 F0163, 1840-1878 F0164, 2466-2621 "
    "F0172 square-aspect/thing-list zone inputs.",
    "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153.",
    "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915,1930-1944,"
    "6507-6548 composition/decoration anchors; D1 side door-facing arrays at "
    "1892-1900 and 1919-1927."
};

static const char s_source_lock_header[] =
    "CSB V1 viewport D1L/D1R F0111 door source lock. Anchors: ReDMCSB "
    "DUNVIEW.C F0111 lines 4218-4337; DUNVIEW.C F0128 lines 8318-8542; "
    "DUNVIEW.C F0104 lines 3113-3156, F0105 lines 3185-3225, F0107 lines "
    "3502-3590, F0108 lines 3940-4011; DUNGEON.C F0163 lines 1769-1838, "
    "F0164 lines 1840-1878, F0172 lines 2466-2621; DEFS.H lines 2088, "
    "2596-2611, 2662, 2668-2677, 4045-4046, 4139-4153; CSB-lineage "
    "Viewport.cpp lines 1192-1209, 1865-1879, 1903-1915, 1930-1944, "
    "6507-6548 plus D1 side door-facing arrays at 1892-1900 and 1919-1927.";

static uint32_t fnv1a(uint32_t hash, uint32_t value)
{
    hash ^= value & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 8) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 16) & 0xffu;
    hash *= 16777619u;
    hash ^= (value >> 24) & 0xffu;
    hash *= 16777619u;
    return hash;
}

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count(void)
{
    return sizeof(s_routes) / sizeof(s_routes[0]);
}

const CSB_V1_ViewportD1LD1RF0111Route *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_at(size_t index)
{
    if (index >= csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count()) {
        return NULL;
    }
    return &s_routes[index];
}

const CSB_V1_ViewportD1LD1RF0111Route *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count();
         ++i) {
        if (s_routes[i].view_square == view_square) return &s_routes[i];
    }
    return NULL;
}

size_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_count(void)
{
    return sizeof(s_steps[0]) / sizeof(s_steps[0][0]);
}

const CSB_V1_ViewportD1LD1RF0111Step *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_at(size_t route_index,
                                                 size_t step_index)
{
    if (route_index >= csb_v1_viewport_d1l_d1r_f0111_door_pc34_route_count()) {
        return NULL;
    }
    if (step_index >= csb_v1_viewport_d1l_d1r_f0111_door_pc34_step_count()) {
        return NULL;
    }
    return &s_steps[route_index][step_index];
}

int csb_v1_viewport_d1l_d1r_f0111_door_pc34_frame_index_for_state(int door_state)
{
    if (door_state == CSB_DOOR_STATE_OPEN) return -1;
    if (door_state > CSB_DOOR_STATE_OPEN && door_state < CSB_DOOR_STATE_CLOSED) {
        return door_state - 1;
    }
    if (door_state == CSB_DOOR_STATE_CLOSED) return CSB_DOOR_STATE_CLOSED;
    if (door_state == CSB_DOOR_STATE_DESTROYED) return CSB_DOOR_STATE_DESTROYED;
    return -1;
}

int csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state)
{
    if (!route) return -1;
    if (door_state == CSB_DOOR_STATE_OPEN) return -1;
    if (door_state > CSB_DOOR_STATE_OPEN && door_state < CSB_DOOR_STATE_CLOSED) {
        return route->door_zone + door_state;
    }
    if (door_state == CSB_DOOR_STATE_CLOSED ||
        door_state == CSB_DOOR_STATE_DESTROYED) {
        return route->door_zone;
    }
    return -1;
}

int csb_v1_viewport_d1l_d1r_f0111_door_pc34_horizontal_zone(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state,
    int right_half)
{
    const int zone = csb_v1_viewport_d1l_d1r_f0111_door_pc34_zone_for_state(
        route, door_state);
    if (zone < 0 || door_state >= CSB_DOOR_STATE_CLOSED) return -1;
    if (right_half) return zone + (3 | CSB_MASK_4000_SHIFT);
    return zone + CSB_C6_UNKNOWN;
}

int csb_v1_viewport_d1l_d1r_f0111_door_pc34_apply_c10_blit(
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[y * source_stride + x];
            if (pixel == (uint8_t)CSB_C10_COLOR_FLESH) continue;
            destination[y * destination_stride + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

uint32_t csb_v1_viewport_d1l_d1r_f0111_door_pc34_render_hash(
    const CSB_V1_ViewportD1LD1RF0111Route *route,
    int door_state)
{
    uint8_t viewport[CSB_RENDER_WIDTH * CSB_RENDER_HEIGHT];
    uint8_t door[6 * 6] = {
        10, 1, 1, 1, 1, 10,
        2, 2, 2, 2, 2, 2,
        3, 3, 10, 10, 3, 3,
        4, 4, 4, 4, 4, 4,
        5, 5, 10, 10, 5, 5,
        10, 6, 6, 6, 6, 10
    };
    int door_x;
    int door_y;
    uint32_t hash = 2166136261u;

    if (!route) return 0;
    for (int i = 0; i < (int)sizeof(viewport); ++i) viewport[i] = 0;

    for (int x = 0; x < CSB_RENDER_WIDTH; ++x) viewport[x] = 20;
    for (int x = 0; x < CSB_RENDER_WIDTH; ++x) {
        viewport[(CSB_RENDER_HEIGHT - 1) * CSB_RENDER_WIDTH + x] = 21;
    }
    viewport[1 * CSB_RENDER_WIDTH + (route->lane < 0 ? 2 : 12)] = 30;
    viewport[7 * CSB_RENDER_WIDTH + (route->lane < 0 ? 3 : 11)] =
        (uint8_t)(route->rear_order & 0xff);
    viewport[2 * CSB_RENDER_WIDTH + (route->lane < 0 ? 4 : 10)] =
        (uint8_t)(route->top_track_zone & 0xff);

    door_x = route->lane < 0 ? 2 : 8;
    door_y = 3;
    if (door_state != CSB_DOOR_STATE_OPEN) {
        (void)csb_v1_viewport_d1l_d1r_f0111_door_pc34_apply_c10_blit(
            door, 6, viewport + door_y * CSB_RENDER_WIDTH + door_x,
            CSB_RENDER_WIDTH, 6, 6);
    }
    viewport[8 * CSB_RENDER_WIDTH + (route->lane < 0 ? 6 : 9)] =
        (uint8_t)(route->front_order & 0xff);

    hash = fnv1a(hash, (uint32_t)route->view_square);
    hash = fnv1a(hash, (uint32_t)door_state);
    hash = fnv1a(hash, (uint32_t)route->rear_order);
    hash = fnv1a(hash, (uint32_t)route->front_order);
    hash = fnv1a(hash, (uint32_t)route->door_zone);
    for (int i = 0; i < (int)sizeof(viewport); ++i) {
        hash = fnv1a(hash, viewport[i]);
    }
    return hash;
}

const CSB_V1_ViewportD1LD1RF0111Evidence *
csb_v1_viewport_d1l_d1r_f0111_door_pc34_evidence(void)
{
    return &s_evidence;
}

const char *csb_v1_viewport_d1l_d1r_f0111_door_pc34_source_lock_header(void)
{
    return s_source_lock_header;
}
