#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_SENTINEL = 0xee,
    PIXEL_TRANSPARENT_COLOR = 10
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

static void verify_center_wall_parity_opaque(DM1_ViewSquareIndex square,
                                             const char *name,
                                             DM1_WallSetIndex expected_wall,
                                             uint8_t base)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[128 * 111];
    uint8_t flipped[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(square);
    const DM1_ViewportWallDrawSpec *spec =
        dm1_viewport_3d_get_wall_draw_spec_for_square(square);
    DM1_ViewportBlitClipGate gate;
    DM1_WallSetIndex selected;
    bool flip_h = false;
    int mirrored_src_x;
    char id[128];

    snprintf(id, sizeof(id), "%s.frame", name);
    check_int(id, frame != NULL, 1);
    snprintf(id, sizeof(id), "%s.spec", name);
    check_int(id, spec != NULL, 1);
    if (!frame || !spec) {
        return;
    }

    /*
     * ReDMCSB source-lock:
     * - DUNVIEW.C:6707-6714,7299-7306,7833-7840 route D3C/D2C/D1C
     *   center walls through G2107_WallSet[C14/C09/C04] with G0076.
     * - DUNVIEW.C:3065-3078 F0101 uses CM1_COLOR_NO_TRANSPARENCY for
     *   center-wall copies, so C10_COLOR_FLESH is copied, not skipped.
     * - DUNVIEW.C:3018-3045 F0099 and 3185-3204 F0105 define the row-local
     *   horizontal flip used when the same G0076 parity flag is active.
     */
    selected = dm1_viewport_3d_select_wall_bitmap(spec, true, &flip_h);
    snprintf(id, sizeof(id), "%s.center_wall", name);
    check_int(id, spec->center_wall ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s.selected_same_center_wall", name);
    check_int(id, (int)selected, (int)expected_wall);
    snprintf(id, sizeof(id), "%s.parity_requests_flip", name);
    check_int(id, flip_h ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s.source_anchor", name);
    check_int(id, strstr(spec->source_lines, "DUNVIEW.C:") != NULL, 1);

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    snprintf(id, sizeof(id), "%s.gate_visible", name);
    check_int(id, gate.visible ? 1 : 0, 1);
    snprintf(id, sizeof(id), "%s.gate_src_x", name);
    check_int(id, gate.src_x, frame->blit_x);
    snprintf(id, sizeof(id), "%s.gate_dst_x", name);
    check_int(id, gate.dst_x, frame->left_x);
    snprintf(id, sizeof(id), "%s.gate_height", name);
    check_int(id, gate.height, frame->height);

    memset(viewport, PIXEL_SENTINEL, sizeof(viewport));
    memset(source, 0x44, sizeof(source));
    memset(flipped, 0, sizeof(flipped));
    for (int y = 0; y < frame->height; ++y) {
        for (int x = 0; x < frame->byte_width; ++x) {
            source[y * frame->byte_width + x] = (uint8_t)(base + ((x + y) & 0x1f));
        }
    }

    mirrored_src_x = frame->byte_width - 1 - gate.src_x;
    source[gate.src_y * frame->byte_width + mirrored_src_x] = PIXEL_TRANSPARENT_COLOR;
    source[gate.src_y * frame->byte_width + mirrored_src_x - 1] = (uint8_t)(base + 0x40);
    source[(gate.src_y + gate.height - 1) * frame->byte_width + mirrored_src_x] =
        (uint8_t)(base + 0x41);

    dm1_viewport_3d_copy_and_flip_h(source, flipped, frame->byte_width, frame->height);
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall_opaque(&state, flipped, frame);

    snprintf(id, sizeof(id), "%s.flipped_c10_copied_opaque", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x],
              PIXEL_TRANSPARENT_COLOR);
    snprintf(id, sizeof(id), "%s.flipped_next_source_pixel", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + 1],
              (uint8_t)(base + 0x40));
    snprintf(id, sizeof(id), "%s.bottom_left_after_flip", name);
    check_int(id, viewport[(gate.dst_y + gate.height - 1) * DM1_VIEWPORT_WIDTH + gate.dst_x],
              (uint8_t)(base + 0x41));
    snprintf(id, sizeof(id), "%s.before_dst_x_untouched", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x - 1],
              PIXEL_SENTINEL);
    snprintf(id, sizeof(id), "%s.after_dst_span_untouched", name);
    check_int(id, viewport[gate.dst_y * DM1_VIEWPORT_WIDTH + gate.dst_x + gate.width],
              PIXEL_SENTINEL);

    printf("centerWallParityOpaque name=%s square=%d wall=%d dst=(%d,%d) src=(%d,%d) size=%dx%d mirroredSrcX=%d source=DUNVIEW.C:3018-3045,3065-3078,3185-3204,6707-6714,7299-7306,7833-7840\n",
           name, (int)square, (int)selected, gate.dst_x, gate.dst_y,
           gate.src_x, gate.src_y, gate.width, gate.height, mirrored_src_x);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_center_wall_parity_opaque_pixel_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:3018-3045,3065-3078,3185-3204,6707-6714,7299-7306,7833-7840\n");

    verify_center_wall_parity_opaque(DM1_VIEW_SQUARE_D3C, "D3C", DM1_WALL_D3C, 0x20);
    verify_center_wall_parity_opaque(DM1_VIEW_SQUARE_D2C, "D2C", DM1_WALL_D2C, 0x40);
    verify_center_wall_parity_opaque(DM1_VIEW_SQUARE_D1C, "D1C", DM1_WALL_D1C, 0x60);

    if (failures) {
        printf("FAIL dm1_v1_center_wall_parity_opaque_pixel_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS dm1_v1_center_wall_parity_opaque_pixel_probe\n");
    return 0;
}
