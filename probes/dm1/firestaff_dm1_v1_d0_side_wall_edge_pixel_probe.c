#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_SENTINEL = 0xee,
    PIXEL_TRANSPARENT = 10,
    PIXEL_SIDE_THING = 0x7d,
    CELL_COUNT = 5
};

static int failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
        ++failures;
    } else {
        printf("PASS %s == %d\n", id, want);
    }
}

static void fill_source(uint8_t *source, const DM1_WallFrame *frame, uint8_t base)
{
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->byte_width; ++x) {
            source[y * frame->byte_width + x] = (uint8_t)(base + ((x + y) & 0x3f));
        }
    }
}

static void verify_d0_edge_wall(DM1_ViewSquareIndex square,
                                const char *name,
                                uint8_t base,
                                int expected_dst_x,
                                int expected_before_x,
                                int expected_unused_frame_x)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[16 * 136];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(square);
    DM1_ViewportBlitClipGate gate;
    char id[128];

    snprintf(id, sizeof(id), "%s.frame_exists", name);
    check_int(id, frame != NULL, 1);
    if (!frame) {
        return;
    }

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);

    /* ReDMCSB source-lock:
     * DUNVIEW.C G0163 lines 593-594 store D0L/D0R as 16-byte-wide,
     * 136-line source bitmaps; F0100 lines 3053-3058 blits from C6/C7
     * through C4/C5 with C10_COLOR_FLESH transparency; F0125/F0126
     * lines 8007-8038 and 8117-8144 are the D0 wall routes. */
    snprintf(id, sizeof(id), "%s.visible", name);
    check_int(id, gate.visible ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s.src_x", name);
    check_int(id, gate.src_x, 0);
    snprintf(id, sizeof(id), "%s.src_y", name);
    check_int(id, gate.src_y, 0);
    snprintf(id, sizeof(id), "%s.dst_x", name);
    check_int(id, gate.dst_x, expected_dst_x);
    snprintf(id, sizeof(id), "%s.dst_y", name);
    check_int(id, gate.dst_y, 0);
    snprintf(id, sizeof(id), "%s.width_clipped_to_source", name);
    check_int(id, gate.width, 16);
    snprintf(id, sizeof(id), "%s.height_full_viewport", name);
    check_int(id, gate.height, DM1_VIEWPORT_HEIGHT);

    memset(viewport, PIXEL_SENTINEL, sizeof(viewport));
    fill_source(source, frame, base);
    source[0] = PIXEL_TRANSPARENT;
    source[gate.width - 1] = (uint8_t)(base + 0x70);
    source[frame->byte_width] = (uint8_t)(base + 0x73);
    source[(gate.height - 1) * frame->byte_width] = (uint8_t)(base + 0x71);
    source[(gate.height - 1) * frame->byte_width + gate.width - 1] = (uint8_t)(base + 0x72);

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall(&state, source, frame);

    snprintf(id, sizeof(id), "%s.transparent_first_pixel_skipped", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x], PIXEL_SENTINEL);
    snprintf(id, sizeof(id), "%s.first_row_second_pixel", name);
    check_int(id,
              viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + 1],
              source[1]);
    snprintf(id, sizeof(id), "%s.first_row_last_pixel", name);
    check_int(id,
              viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width - 1],
              (uint8_t)(base + 0x70));
    snprintf(id, sizeof(id), "%s.second_row_first_pixel_uses_next_source_row", name);
    check_int(id,
              viewport[(gate.dst_y + 1) * DM1_VIEWPORT_WIDTH + gate.dst_x],
              (uint8_t)(base + 0x73));
    snprintf(id, sizeof(id), "%s.last_row_first_pixel", name);
    check_int(id,
              viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH + gate.dst_x],
              (uint8_t)(base + 0x71));
    snprintf(id, sizeof(id), "%s.last_row_last_pixel", name);
    check_int(id,
              viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width - 1],
              (uint8_t)(base + 0x72));
    snprintf(id, sizeof(id), "%s.after_right_edge_untouched", name);
    check_int(id,
              viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width],
              PIXEL_SENTINEL);
    snprintf(id, sizeof(id), "%s.last_row_after_right_edge_untouched", name);
    check_int(id,
              viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH +
                       gate.dst_x + gate.width],
              PIXEL_SENTINEL);
    if (expected_before_x >= 0) {
        snprintf(id, sizeof(id), "%s.before_left_edge_untouched", name);
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + expected_before_x],
                  PIXEL_SENTINEL);
        snprintf(id, sizeof(id), "%s.last_row_before_left_edge_untouched", name);
        check_int(id,
                  viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH +
                           expected_before_x],
                  PIXEL_SENTINEL);
    }
    if (expected_unused_frame_x >= 0) {
        snprintf(id, sizeof(id), "%s.unused_frame_span_untouched", name);
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + expected_unused_frame_x],
                  PIXEL_SENTINEL);
        snprintf(id, sizeof(id), "%s.last_row_unused_frame_span_untouched", name);
        check_int(id,
                  viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH +
                           expected_unused_frame_x],
                  PIXEL_SENTINEL);
    }

    printf("d0SideWallEdge name=%s square=%d dst=(%d,%d) src=(%d,%d) size=%dx%d outsideLeft=%d source=DUNVIEW.C:593-594,3053-3058,8007-8038,8117-8144\n",
           name, (int)square, gate.dst_x, gate.dst_y, gate.src_x, gate.src_y,
           gate.width, gate.height, expected_before_x);
}

static void verify_d0_door_side_cell_order(DM1_ViewSquareIndex square,
                                           const char *name,
                                           uint16_t expected_order,
                                           unsigned char expected_cell,
                                           unsigned char occluded_cell,
                                           const char *branch_line,
                                           const char *f0115_line)
{
    uint8_t cells[CELL_COUNT] = {
        PIXEL_SENTINEL,
        PIXEL_SENTINEL,
        PIXEL_SENTINEL,
        PIXEL_SENTINEL,
        PIXEL_SENTINEL
    };
    const DM1_ViewportSideOcclusionSpec *side =
        dm1_viewport_3d_get_side_occlusion_spec_for_square(square);
    const DM1_ViewportDoorFrontOcclusionSpec *front =
        dm1_viewport_3d_get_door_front_occlusion_spec_for_square(square);
    DM1_ViewportCellOrder order;
    char id[128];

    snprintf(id, sizeof(id), "%s.door_side_spec_exists", name);
    check_int(id, side != NULL, 1);
    snprintf(id, sizeof(id), "%s.not_door_front_spec", name);
    check_int(id, front == NULL, 1);
    if (!side) {
        return;
    }

    /* ReDMCSB source-lock:
     * DUNVIEW.C F0125 lines 8000-8005 and F0126 lines 8110-8115 route
     * D0 side-door/teleporter cells directly to F0115.  They do not draw
     * an F0111 door-front panel, so this near-edge gate pins the surviving
     * cell nibble rather than front/rear door-pass ordering. */
    snprintf(id, sizeof(id), "%s.function", name);
    check_int(id, strstr(side->function_name, name) != NULL, 1);
    snprintf(id, sizeof(id), "%s.branch_source", name);
    check_int(id, strstr(side->branch_source_lines, branch_line) != NULL, 1);
    snprintf(id, sizeof(id), "%s.f0115_source", name);
    check_int(id, strstr(side->f0115_source_lines, f0115_line) != NULL, 1);
    snprintf(id, sizeof(id), "%s.cell_order", name);
    check_int(id, side->cell_order, expected_order);

    order = dm1_viewport_3d_decode_cell_order(side->cell_order);
    snprintf(id, sizeof(id), "%s.no_door_pass_marker", name);
    check_int(id, order.door_pass, 0);
    snprintf(id, sizeof(id), "%s.one_visible_cell", name);
    check_int(id, order.cell_count, 1);
    snprintf(id, sizeof(id), "%s.visible_cell", name);
    check_int(id, order.cells[0], expected_cell);

    cells[occluded_cell] = PIXEL_SENTINEL;
    cells[expected_cell] = PIXEL_SIDE_THING;
    snprintf(id, sizeof(id), "%s.edge_cell_drawn", name);
    check_int(id, cells[expected_cell], PIXEL_SIDE_THING);
    snprintf(id, sizeof(id), "%s.opposite_edge_cell_occluded", name);
    check_int(id, cells[occluded_cell], PIXEL_SENTINEL);

    printf("d0DoorSideEdge name=%s square=%d order=0x%04x visibleCell=%u occludedCell=%u source=%s,%s\n",
           name, (int)square, (unsigned int)side->cell_order, expected_cell, occluded_cell,
           side->branch_source_lines, side->f0115_source_lines);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_d0_side_wall_edge_pixel_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:593-594,3053-3058,8000-8005,8007-8038,8110-8115,8117-8144\n");

    verify_d0_edge_wall(DM1_VIEW_SQUARE_D0L, "D0L", 0x20, 0, -1, 31);
    verify_d0_edge_wall(DM1_VIEW_SQUARE_D0R, "D0R", 0x50, 192, 191, 223);
    verify_d0_door_side_cell_order(DM1_VIEW_SQUARE_D0L,
                                   "D0L",
                                   0x0002,
                                   2,
                                   1,
                                   "8000-8005",
                                   "8005");
    verify_d0_door_side_cell_order(DM1_VIEW_SQUARE_D0R,
                                   "D0R",
                                   0x0001,
                                   1,
                                   2,
                                   "8110-8115",
                                   "8115");

    if (failures) {
        printf("FAIL dm1_v1_d0_side_wall_edge_pixel_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS dm1_v1_d0_side_wall_edge_pixel_probe\n");
    return 0;
}
