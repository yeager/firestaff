#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_BACKGROUND = 0x33,
    PIXEL_TRANSPARENT = 10,
    PIXEL_SOURCE_ORIGIN_SENTINEL = 0xa5,
    PIXEL_SOURCE_PRECLIP_SENTINEL = 0xb6
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
    int width = frame->byte_width;
    int height = frame->height;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            source[y * width + x] = (uint8_t)(base + ((x + y) & 0x1f));
        }
    }
}

static void verify_side_wall_pixels(DM1_ViewSquareIndex square,
                                    const char *name,
                                    uint8_t base,
                                    const char *source_anchor)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[128 * 136];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame;
    DM1_ViewportBlitClipGate gate;
    char id[128];

    frame = dm1_viewport_3d_get_wall_frame(square);
    snprintf(id, sizeof(id), "%s.frame", name);
    check_int(id, frame != NULL, 1);
    if (!frame) {
        return;
    }

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    snprintf(id, sizeof(id), "%s.visible", name);
    check_int(id, gate.visible ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s.source_anchor", name);
    check_int(id, strstr(source_anchor, "DUNVIEW.C:") != NULL, 1);

    memset(viewport, PIXEL_BACKGROUND, sizeof(viewport));
    memset(source, PIXEL_TRANSPARENT, sizeof(source));
    fill_source(source, frame, base);

    /* ReDMCSB F0100 passes frame C6/C7 as the first source pixel and C4/C5
     * as the source size, while C10_COLOR_FLESH remains transparent.
     * Source: DUNVIEW.C:3048-3058; G0163 frame data at DUNVIEW.C:581-594. */
    source[0] = PIXEL_SOURCE_ORIGIN_SENTINEL;
    if (gate.src_x > 0) {
        source[gate.src_y * frame->byte_width + gate.src_x - 1] =
            PIXEL_SOURCE_PRECLIP_SENTINEL;
    }
    source[gate.src_y * frame->byte_width + gate.src_x] = PIXEL_TRANSPARENT;
    source[(gate.src_y + gate.height - 1) * frame->byte_width + gate.src_x + gate.width - 1] =
        (uint8_t)(base + 0x40);

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall(&state, source, frame);

    snprintf(id, sizeof(id), "%s.first_visible_pixel_transparent_skip", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x], PIXEL_BACKGROUND);
    snprintf(id, sizeof(id), "%s.second_visible_pixel_source_offset", name);
    check_int(id,
              viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + 1],
              source[gate.src_y * frame->byte_width + gate.src_x + 1]);
    snprintf(id, sizeof(id), "%s.source_origin_not_leaked", name);
    if ((gate.src_x > 0 || gate.src_y > 0) && gate.width > 1) {
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + 1] !=
                      PIXEL_SOURCE_ORIGIN_SENTINEL,
                  1);
    } else {
        check_int(id, 1, 1);
    }
    snprintf(id, sizeof(id), "%s.preclip_source_not_leaked", name);
    if (gate.src_x > 0) {
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x] !=
                      PIXEL_SOURCE_PRECLIP_SENTINEL,
                  1);
    } else {
        check_int(id, 1, 1);
    }
    snprintf(id, sizeof(id), "%s.last_visible_pixel", name);
    check_int(id,
              viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width - 1],
              (uint8_t)(base + 0x40));
    snprintf(id, sizeof(id), "%s.before_left_edge_untouched", name);
    if (gate.dst_x > 0) {
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x - 1],
                  PIXEL_BACKGROUND);
    } else {
        check_int(id, 1, 1);
    }
    snprintf(id, sizeof(id), "%s.after_right_edge_untouched", name);
    if (gate.dst_x + gate.width < DM1_VIEWPORT_WIDTH) {
        check_int(id,
                  viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width],
                  PIXEL_BACKGROUND);
    } else {
        check_int(id, 1, 1);
    }

    printf("sideWallPixel name=%s square=%d dst=(%d,%d) src=(%d,%d) size=%dx%d source=%s\n",
           name, (int)square, gate.dst_x, gate.dst_y, gate.src_x, gate.src_y,
           gate.width, gate.height, source_anchor);
}

static void verify_d1l_fully_clipped(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[128 * 136];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1L);
    DM1_ViewportBlitClipGate gate;

    check_int("D1L.frame", frame != NULL, 1);
    if (!frame) {
        return;
    }

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("D1L.clip_gate_invisible", gate.visible ? 1 : 0, 0);

    memset(viewport, PIXEL_BACKGROUND, sizeof(viewport));
    memset(source, 0x7a, sizeof(source));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall(&state, source, frame);

    check_int("D1L.left_edge_untouched", viewport[9 * DM1_VIEWPORT_WIDTH + 0], PIXEL_BACKGROUND);
    check_int("D1L.right_edge_untouched", viewport[119 * DM1_VIEWPORT_WIDTH + 63], PIXEL_BACKGROUND);
    printf("sideWallPixel name=D1L square=%d clipped=1 source=DUNVIEW.C:589-590,7445-7455\n",
           (int)DM1_VIEW_SQUARE_D1L);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_side_wall_pixel_clip_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:579-594,3048-3058,6837-6896,8448-8462,6406-6437,6545-6573,6954-6964,7105-7115,7445-7455,7613-7623\n");

    verify_side_wall_pixels(DM1_VIEW_SQUARE_D3L2, "D3L2", 0x08, "DUNVIEW.C:579,8448-8451");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D3R2, "D3R2", 0x0c, "DUNVIEW.C:580,8456-8462");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D3L, "D3L", 0x10, "DUNVIEW.C:584,6406-6437");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D3R, "D3R", 0x18, "DUNVIEW.C:585,6545-6573");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D2L2, "D2L2", 0x1c, "DUNVIEW.C:6837-6865");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D2R2, "D2R2", 0x1e, "DUNVIEW.C:6868-6896");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D2L, "D2L", 0x20, "DUNVIEW.C:587,6954-6964");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D2R, "D2R", 0x40, "DUNVIEW.C:588,7105-7115");
    verify_side_wall_pixels(DM1_VIEW_SQUARE_D1R, "D1R", 0x60, "DUNVIEW.C:591,7613-7623");
    verify_d1l_fully_clipped();

    if (failures) {
        printf("FAIL dm1_v1_side_wall_pixel_clip_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS dm1_v1_side_wall_pixel_clip_probe\n");
    return 0;
}
