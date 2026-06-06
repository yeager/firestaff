#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    PIXEL_SENTINEL = 0xee,
    PIXEL_SOURCE = 0x42
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

static void verify_no_f0128_d1_lateral2_step(void)
{
    int found_d1l2 = 0;
    int found_d1r2 = 0;
    size_t count = dm1_viewport_3d_draw_order_count();

    /* ReDMCSB source-lock: DUNVIEW.C F0128 lines 8500-8508 route the
     * only lateral-2 side helpers at depth 2 (F0678/F0679).  Lines
     * 8522-8533 then route D1 as D1L, D1R, D1C only; there is no
     * D1L2/D1R2 callback between those ranges. */
    check_int("f0128.draw_order_count", (int)count, 19);
    for (size_t i = 0; i < count; ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(i);
        if (!step) {
            check_int("f0128.step_nonnull", 0, 1);
            continue;
        }
        check_source_anchor("f0128.step_source_anchor", step->source_lines, "DUNVIEW.C:");
        if (step->rel_depth == 1 && step->rel_lateral == -2) {
            found_d1l2 = 1;
        }
        if (step->rel_depth == 1 && step->rel_lateral == 2) {
            found_d1r2 = 1;
        }
    }

    check_int("f0128.no_d1l2_step", found_d1l2, 0);
    check_int("f0128.no_d1r2_step", found_d1r2, 0);
    check_int("f0128.d1l_step",
              dm1_viewport_3d_get_draw_order_step(13)->square == DM1_VIEW_SQUARE_D1L, 1);
    check_int("f0128.d1r_step",
              dm1_viewport_3d_get_draw_order_step(14)->square == DM1_VIEW_SQUARE_D1R, 1);
    check_int("f0128.d1c_step",
              dm1_viewport_3d_get_draw_order_step(15)->square == DM1_VIEW_SQUARE_D1C, 1);
}

static void verify_pc34_extra_lane_rejects_depth1(void)
{
    int x = 99;
    int y = 99;
    int found_left;
    int found_right;
    uint8_t mask_left;
    uint8_t mask_right;

    /* ReDMCSB source-lock: DEFS.H lines 2578-2589 define only D1C/D1L/D1R
     * for the ordinary wall frame index set, while DUNVIEW.C F0128 lines
     * 8478-8508 expose lateral-2 helper calls only for D3 and D2. */
    found_left = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 1, -2, &x, &y);
    check_int("pc34_extra.d1l2_rejected", found_left, 0);
    check_int("pc34_extra.d1l2_x_untouched", x, 99);
    check_int("pc34_extra.d1l2_y_untouched", y, 99);

    found_right = dm1_get_pc34_extra_side_wall_coords(2, 2, DM1_DIR_NORTH, 1, 2, &x, &y);
    check_int("pc34_extra.d1r2_rejected", found_right, 0);
    check_int("pc34_extra.d1r2_x_untouched", x, 99);
    check_int("pc34_extra.d1r2_y_untouched", y, 99);

    mask_left = dm1_compute_pc34_extra_side_wall_visibility(1, -2, DM1_VP_ELEMENT_WALL, DM1_DIR_NORTH);
    mask_right = dm1_compute_pc34_extra_side_wall_visibility(1, 2, DM1_VP_ELEMENT_WALL, DM1_DIR_NORTH);
    check_int("pc34_extra.d1l2_no_mask", mask_left, 0);
    check_int("pc34_extra.d1r2_no_mask", mask_right, 0);
}

static void verify_absent_pixel_slice_is_no_write(void)
{
    uint8_t viewport[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t before[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    uint8_t source[128 * 111];
    DM1_Viewport3DState state;
    const DM1_WallFrame *d1l_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1L);
    const DM1_WallFrame *d1r_frame = dm1_viewport_3d_get_wall_frame(DM1_VIEW_SQUARE_D1R);
    const DM1_WallFrame *d1l2_frame = dm1_viewport_3d_get_wall_frame((DM1_ViewSquareIndex)-201);
    const DM1_WallFrame *d1r2_frame = dm1_viewport_3d_get_wall_frame((DM1_ViewSquareIndex)-202);

    /* ReDMCSB source-lock: DUNVIEW.C F0122/F0123 lines 7436-7460 and
     * 7604-7628 are the real nearest side-wall pixel paths.  Because
     * F0128 lines 8522-8533 call only those D1 side helpers plus D1C,
     * the synthetic D1L2/D1R2 slots must have no wall frame, no wall spec,
     * and no writeable pixel rectangle. */
    memset(viewport, PIXEL_SENTINEL, sizeof(viewport));
    memcpy(before, viewport, sizeof(before));
    memset(source, PIXEL_SOURCE, sizeof(source));
    dm1_viewport_3d_init(&state, viewport, DM1_VIEWPORT_WIDTH);

    check_int("pixel_slice.d1l_frame_exists", d1l_frame != NULL, 1);
    check_int("pixel_slice.d1r_frame_exists", d1r_frame != NULL, 1);
    check_int("pixel_slice.d1l2_frame_absent", d1l2_frame == NULL, 1);
    check_int("pixel_slice.d1r2_frame_absent", d1r2_frame == NULL, 1);
    check_int("pixel_slice.d1l2_spec_absent",
              dm1_viewport_3d_get_wall_draw_spec_for_square((DM1_ViewSquareIndex)-201) == NULL, 1);
    check_int("pixel_slice.d1r2_spec_absent",
              dm1_viewport_3d_get_wall_draw_spec_for_square((DM1_ViewSquareIndex)-202) == NULL, 1);

    dm1_viewport_3d_draw_wall(&state, source, d1l2_frame);
    dm1_viewport_3d_draw_wall(&state, source, d1r2_frame);
    check_int("pixel_slice.viewport_unchanged_after_absent_lanes",
              memcmp(viewport, before, sizeof(viewport)) == 0, 1);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_d1l2_d1r2_absence_pixel_slice_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:8478-8508,8522-8533; DEFS.H:2578-2589\n");

    verify_no_f0128_d1_lateral2_step();
    verify_pc34_extra_lane_rejects_depth1();
    verify_absent_pixel_slice_is_no_write();

    if (failures) {
        printf("FAIL dm1_v1_d1l2_d1r2_absence_pixel_slice_probe failures=%d\n", failures);
        return 1;
    }

    printf("PASS dm1_v1_d1l2_d1r2_absence_pixel_slice_probe\n");
    return 0;
}
