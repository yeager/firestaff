#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_SENTINEL = 0xee
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

static void check_source_anchor(const char *id, const char *text, const char *needle)
{
    check_int(id, text && needle && strstr(text, needle) ? 1 : 0, 1);
}

static void verify_d0c_absent_wall_draw_path(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t before[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[16];
    DM1_Viewport3DState state;
    DM1_ViewportBlitClipGate gate;
    const DM1_WallFrame *frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D0C);
    const DM1_ViewportWallDrawSpec *spec = dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D0C);
    const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(18);

    check_int("d0c.absence.frame_exists", frame != NULL, 1);
    check_int("d0c.absence.spec_absent", spec == NULL, 1);

    if (!frame) {
        return;
    }

    check_int("d0c.absence.frame_width_zero", (int)frame->byte_width, 0);
    check_int("d0c.absence.frame_height_zero", (int)frame->height, 0);
    check_int("d0c.absence.draw_order_step_18_exists", step != NULL, 1);
    if (step) {
        check_int("d0c.absence.draw_order_step_18_is_d0c", step->square == DM1_VIEW_SQUARE_D0C, 1);
        check_source_anchor("d0c.absence.step_18_source", step->source_lines, "DUNVIEW.C:8542");
    }

    gate = dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);
    check_int("d0c.absence.gate_visible", gate.visible ? 1 : 0, 0);
    check_int("d0c.absence.gate_src_x_zero", gate.src_x, 0);
    check_int("d0c.absence.gate_src_y_zero", gate.src_y, 0);
    check_int("d0c.absence.gate_dst_x_zero", gate.dst_x, 0);
    check_int("d0c.absence.gate_dst_y_zero", gate.dst_y, 0);
    check_int("d0c.absence.gate_width_zero", gate.width, 0);
    check_int("d0c.absence.gate_height_zero", gate.height, 0);

    memset(viewport, PIXEL_SENTINEL, sizeof(viewport));
    memcpy(before, viewport, sizeof(before));
    memset(source, 0x33, sizeof(source));

    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);
    dm1_viewport_3d_draw_wall(&state, source, frame);
    check_int("d0c.absence.viewport_unchanged", memcmp(viewport, before, sizeof(viewport)) == 0, 1);

    printf("d0c_absence frame=%p spec=%s dims=%ux%u\n",
           (const void *)frame,
           spec == NULL ? "null" : "present",
           (unsigned int)frame->byte_width,
           (unsigned int)frame->height);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_d0c_wall_absence_pixel_slice_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:8542,8117-8144,8185-8240,8007-8038\n");

    verify_d0c_absent_wall_draw_path();

    if (failures) {
        printf("FAIL firestaff_dm1_v1_d0c_wall_absence_pixel_slice_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS firestaff_dm1_v1_d0c_wall_absence_pixel_slice_probe\n");
    return 0;
}
