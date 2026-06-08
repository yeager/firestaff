#include "csb/csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_D2L2_VIEW_SQUARE = 9,
    CSB_D2R2_VIEW_SQUARE = 10,
    CSB_D2L2_ORDER = 8,
    CSB_D2R2_ORDER = 9,
    CSB_D2_DEPTH = 2,
    CSB_D2L2_LANE = -2,
    CSB_D2R2_LANE = 2,
    CSB_C707_ZONE_WALL_D2L2 = 707,
    CSB_C708_ZONE_WALL_D2R2 = 708,
    CSB_C2500_ZONE_OBJECT_BASE = 2500,
    CSB_C2900_ZONE_PROJECTILE_BASE = 2900,
    CSB_D3L2_DOOR_ZONE_BASE = 3700,
    CSB_DOOR_STATE_OPEN = 0,
    CSB_DOOR_STATE_PARTLY_ONE = 1,
    CSB_DOOR_STATE_PARTLY_TWO = 2,
    CSB_DOOR_STATE_PARTLY_THREE = 3,
    CSB_DOOR_STATE_CLOSED = 4,
    CSB_DOOR_STATE_DESTROYED = 5,
    CSB_C6_UNKNOWN = 6,
    CSB_FINAL_HALF_OFFSET = 3,
    CSB_MASK0X4000 = 0x4000,
    CSB_C10_COLOR_FLESH = 10,
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_FRAMEBUFFER_WIDTH = 320,
    CSB_FRAMEBUFFER_HEIGHT = 200,
    CSB_C03_CLIPPED_WIDTH = 48,
    CSB_C03_CLIPPED_HEIGHT = 40,
    CSB_C03_PARENT_RECORD = 129,
    CSB_C03_CLIP_RECORD = 126,
    CSB_C03_FRAME_X = 24,
    CSB_C03_FRAME_Y = 28,
    CSB_BRANCH_OPEN = 0,
    CSB_BRANCH_PARTLY_OPEN = 1,
    CSB_BRANCH_CLOSED = 2,
    CSB_BRANCH_DESTROYED = 3,
    CSB_BRANCH_INVALID = -1
};

static const char s_source_evidence[] =
    "contract-only synthetic source-lock gate; no real-asset pixel parity and "
    "no game-data load. ReDMCSB DUNVIEW.C:4218-4337 "
    "F0111_DUNGEONVIEW_DrawDoor anchors the branch: line 4248 skips C0 open; "
    "4297-4299 draws C4 closed through ClosedOrDestroyed; 4301-4304 draws "
    "C5 destroyed through destroyed mask + ClosedOrDestroyed; 4308-4313 "
    "selects partly-open Vertical/LeftHorizontal/RightHorizontal door-frame "
    "bitmaps; 4317-4325 performs the PC34 horizontal partly-open zone math; "
    "4334 blits with C10. ReDMCSB DUNVIEW.C:8503-8508 F0128 dispatches "
    "D2L2/D2R2 view-depth 2 lane -2/+2 through F0678/F0679. "
    "DUNVIEW.C:6837-6896 binds D2L2/D2R2 wall/teleporter squares to "
    "C707/C708 and returns wall cases before F0111, making this gate "
    "non-duplicative with D2L2/D2R2 wall and direct partly-open/front-clipped "
    "F0111 gates. DEFS.H:2088 C10_COLOR_FLESH, 2605-2606 C09/C10 D2L2/D2R2, "
    "3508 C6_UNKNOWN, 3516 MASK0x4000, 4047-4048 C707/C708, 4228-4230 "
    "C2500_ZONE_/C2900_ZONE_. COORD.C:788-797 C3700 state records and "
    "1556-1559 C03 48x40 clip through parent 129 at x=24,y=28 anchor the "
    "synthetic clipped 224x136 viewport write inside a 320x200 buffer. "
    "CSB Viewport.cpp:1903-1906 is recorded as CSB-lineage room-object "
    "overlay evidence only. Requested C2600_DOOR_PARTLY_OPEN_BITMAP is absent "
    "from available ReDMCSB Common/Source; F0111:4308-4313 is the cited "
    "partly-open bitmap-selection source anchor.";

#define COMMON_SPEC(square, order, lane, wall_zone, name, d2_anchor) \
    { \
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, square, order, CSB_D2_DEPTH, \
        lane, wall_zone, CSB_ABSENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, \
        CSB_PRESENT, CSB_C2500_ZONE_OBJECT_BASE, CSB_C2900_ZONE_PROJECTILE_BASE, \
        CSB_D3L2_DOOR_ZONE_BASE, CSB_DOOR_STATE_OPEN, CSB_DOOR_STATE_PARTLY_ONE, \
        CSB_DOOR_STATE_PARTLY_TWO, CSB_DOOR_STATE_PARTLY_THREE, \
        CSB_DOOR_STATE_CLOSED, CSB_DOOR_STATE_DESTROYED, CSB_C6_UNKNOWN, \
        CSB_C6_UNKNOWN, CSB_FINAL_HALF_OFFSET, CSB_MASK0X4000, \
        CSB_C10_COLOR_FLESH, CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT, \
        CSB_FRAMEBUFFER_WIDTH, CSB_FRAMEBUFFER_HEIGHT, CSB_C03_CLIPPED_WIDTH, \
        CSB_C03_CLIPPED_HEIGHT, CSB_C03_PARENT_RECORD, CSB_C03_CLIP_RECORD, \
        CSB_C03_FRAME_X, CSB_C03_FRAME_Y, CSB_ABSENT, name, \
        "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor", \
        "ReDMCSB DUNVIEW.C:8503-8508 F0128_DUNGEONVIEW_Draw_CPSF", \
        d2_anchor, \
        "ReDMCSB DEFS.H:2088,2605-2606,3508,3516,4047-4048,4228-4230", \
        "ReDMCSB COORD.C:788-797 and 1556-1559", \
        "CSB Viewport.cpp:1903-1906 room-object overlay binding", \
        "C2600_DOOR_PARTLY_OPEN_BITMAP absent; use ReDMCSB DUNVIEW.C:4308-4313" \
    }

static const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 s_specs[] = {
    COMMON_SPEC(CSB_D2L2_VIEW_SQUARE, CSB_D2L2_ORDER, CSB_D2L2_LANE,
                CSB_C707_ZONE_WALL_D2L2,
                "D2L2 F0111 partly-open door-frame contract",
                "ReDMCSB DUNVIEW.C:6837-6865 F0678_DrawD2L2"),
    COMMON_SPEC(CSB_D2R2_VIEW_SQUARE, CSB_D2R2_ORDER, CSB_D2R2_LANE,
                CSB_C708_ZONE_WALL_D2R2,
                "D2R2 F0111 partly-open door-frame contract",
                "ReDMCSB DUNVIEW.C:6868-6896 F0679_DrawD2R2")
};

#undef COMMON_SPEC

size_t csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(
    int view_square)
{
    for (size_t i = 0;
         i < csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34();
         ++i) {
        if (s_specs[i].view_square == view_square) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state)
{
    if (!spec) return CSB_BRANCH_INVALID;
    if (door_state == spec->open_state) return CSB_BRANCH_OPEN;
    if (door_state == spec->closed_state) return CSB_BRANCH_CLOSED;
    if (door_state == spec->destroyed_state) return CSB_BRANCH_DESTROYED;
    if (door_state >= spec->partly_open_state_one &&
        door_state <= spec->partly_open_state_three) {
        return CSB_BRANCH_PARTLY_OPEN;
    }
    return CSB_BRANCH_INVALID;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    if (csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) != CSB_BRANCH_PARTLY_OPEN) {
        return -1;
    }
    if (!horizontal_door) return -1;
    return spec->door_zone_base + door_state + spec->first_half_zone_offset;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int horizontal_door)
{
    const int branch =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(
            spec, door_state);
    int zone;

    if (!spec) return -1;
    if (branch == CSB_BRANCH_OPEN || branch == CSB_BRANCH_INVALID) return -1;
    if (branch == CSB_BRANCH_CLOSED || branch == CSB_BRANCH_DESTROYED) {
        return spec->door_zone_base;
    }

    zone = spec->door_zone_base + door_state;
    if (horizontal_door) {
        zone += spec->final_half_zone_offset;
        zone |= spec->final_half_mask;
    }
    return zone;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec,
    int door_state,
    int dest_x,
    int dest_y,
    const uint8_t *source,
    int source_width,
    int source_height,
    int source_stride,
    uint8_t *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    int *out_c10_skipped,
    int *out_left_edge_writes,
    int *out_right_edge_writes)
{
    int copied = 0;
    int skipped = 0;
    int left_writes = 0;
    int right_writes = 0;
    const int viewport_left = 0;
    const int viewport_top = 0;
    int viewport_right;
    int viewport_bottom;

    if (!spec || !source || !framebuffer) return -1;
    if (source_width <= 0 || source_height <= 0) return -1;
    if (source_stride < source_width) return -1;
    if (source_width > spec->clipped_width || source_height > spec->clipped_height) {
        return -1;
    }
    if (framebuffer_width != spec->framebuffer_width ||
        framebuffer_height != spec->framebuffer_height) {
        return -1;
    }
    if (csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_branch_pc34(
            spec, door_state) == CSB_BRANCH_OPEN) {
        return 0;
    }

    viewport_right = spec->viewport_width - 1;
    viewport_bottom = spec->viewport_height - 1;

    for (int y = 0; y < source_height; ++y) {
        const int fb_y = dest_y + y;
        if (fb_y < viewport_top || fb_y > viewport_bottom) continue;
        for (int x = 0; x < source_width; ++x) {
            const int fb_x = dest_x + x;
            uint8_t pixel;
            if (fb_x < viewport_left || fb_x > viewport_right) continue;
            pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) {
                ++skipped;
                continue;
            }
            framebuffer[(fb_y * framebuffer_width) + fb_x] = pixel;
            ++copied;
            if (fb_x == viewport_left) ++left_writes;
            if (fb_x == viewport_right) ++right_writes;
        }
    }

    if (out_c10_skipped) *out_c10_skipped = skipped;
    if (out_left_edge_writes) *out_left_edge_writes = left_writes;
    if (out_right_edge_writes) *out_right_edge_writes = right_writes;
    return copied;
}

int csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_probe_pc34_compat(
    CSB_V1_ViewportD2L2D2R2F0111PartlyOpenDoorProbePc34 *out_probe)
{
    uint8_t source[CSB_C03_CLIPPED_WIDTH * CSB_C03_CLIPPED_HEIGHT];
    uint8_t framebuffer[CSB_FRAMEBUFFER_WIDTH * CSB_FRAMEBUFFER_HEIGHT];
    const CSB_V1_ViewportD2L2D0R2F0111PartlyOpenDoorSpecPc34 *spec =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_for_square_pc34(
            CSB_D2L2_VIEW_SQUARE);
    int skipped = 0;
    int left_writes = 0;
    int right_writes = 0;
    int copied;

    if (!out_probe || !spec) return -1;

    for (int i = 0; i < (CSB_FRAMEBUFFER_WIDTH * CSB_FRAMEBUFFER_HEIGHT); ++i) {
        framebuffer[i] = 0xEE;
    }
    for (int y = 0; y < CSB_C03_CLIPPED_HEIGHT; ++y) {
        for (int x = 0; x < CSB_C03_CLIPPED_WIDTH; ++x) {
            source[(y * CSB_C03_CLIPPED_WIDTH) + x] =
                (uint8_t)(((x + y) % 5) == 0 ? CSB_C10_COLOR_FLESH : 0x31);
        }
    }

    copied =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_synthetic_blit_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, -3, 96, source,
            CSB_C03_CLIPPED_WIDTH, CSB_C03_CLIPPED_HEIGHT,
            CSB_C03_CLIPPED_WIDTH, framebuffer, CSB_FRAMEBUFFER_WIDTH,
            CSB_FRAMEBUFFER_HEIGHT, &skipped, &left_writes, &right_writes);

    out_probe->route_count =
        (int)csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_spec_count_pc34();
    out_probe->assertions = 0;
    out_probe->failures = copied < 0 ? 1 : 0;
    out_probe->c10_skipped_pixels = skipped;
    out_probe->copied_pixels = copied;
    out_probe->left_edge_writes = left_writes;
    out_probe->right_edge_writes = right_writes;
    out_probe->first_half_zone =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_first_half_zone_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->final_half_zone =
        csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_final_half_zone_pc34(
            spec, CSB_DOOR_STATE_PARTLY_TWO, CSB_PRESENT);
    out_probe->clipped_write_inside_224x136 =
        copied > 0 && framebuffer[(96 * CSB_FRAMEBUFFER_WIDTH)] != 0xEE;
    out_probe->no_real_asset_pixel_parity = CSB_PRESENT;
    return out_probe->failures ? -1 : 0;
}

const char *
csb_v1_viewport_d2l2_d2r2_f0111_partly_open_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
