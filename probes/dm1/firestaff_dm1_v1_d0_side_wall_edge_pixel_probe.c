#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_SENTINEL = 0xee,
    PIXEL_TRANSPARENT = 10
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

int main(void)
{
    printf("probe=firestaff_dm1_v1_d0_side_wall_edge_pixel_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:593-594,3053-3058,8007-8038,8117-8144\n");

    verify_d0_edge_wall(DM1_VIEW_SQUARE_D0L, "D0L", 0x20, 0, -1, 31);
    verify_d0_edge_wall(DM1_VIEW_SQUARE_D0R, "D0R", 0x50, 192, 191, 223);

    if (failures) {
        printf("FAIL dm1_v1_d0_side_wall_edge_pixel_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS dm1_v1_d0_side_wall_edge_pixel_probe\n");
    return 0;
}
