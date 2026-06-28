#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    PIXEL_OWNER_BACKGROUND = 0,
    PIXEL_OWNER_D4_OBJECT = 1,
    PIXEL_OWNER_D3L2_SIDE_WALL = 2,
    PIXEL_OWNER_D3L_WALL = 3,
    PIXEL_OWNER_D3C_WALL = 4,
    PIXEL_OWNER_D3R2_SIDE_WALL = 5,
    PIXEL_OWNER_D3R_WALL = 6
} PixelOwner;

typedef struct {
    DM1_ViewSquareIndex square;
    PixelOwner owner;
    const char *phase;
} PixelLane;

static int failures = 0;

static void check_int(const char *id, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", id, got, want);
        ++failures;
    }
}

static void check_bool(const char *id, bool got, bool want)
{
    check_int(id, got ? 1 : 0, want ? 1 : 0);
}

static void check_contains(const char *id, const char *text, const char *needle)
{
    check_bool(id, text && needle && strstr(text, needle), true);
}

static int draw_order_index(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_draw_order_count(); ++i) {
        const DM1_ViewportDrawStep *step = dm1_viewport_3d_get_draw_order_step(i);
        if (step && step->square == square) {
            return (int)i;
        }
    }
    return -1;
}

static void write_pixel(PixelLane *pixel,
                        DM1_ViewSquareIndex square,
                        PixelOwner owner,
                        const char *phase)
{
    pixel->square = square;
    pixel->owner = owner;
    pixel->phase = phase;
}

static void verify_far_left_object_hidden_by_d3_side_walls(void)
{
    const DM1_ViewportFarObjectPassSpec *d4l =
        dm1_viewport_3d_get_far_object_pass_spec_for_square(DM1_VIEW_SQUARE_D4L);
    const DM1_ViewportWallDrawSpec *d3l2 =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3L2);
    const DM1_ViewportWallDrawSpec *d3l =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3L);
    const int d4l_order = draw_order_index(DM1_VIEW_SQUARE_D4L);
    const int d3l2_order = draw_order_index(DM1_VIEW_SQUARE_D3L2);
    const int d3l_order = draw_order_index(DM1_VIEW_SQUARE_D3L);
    PixelLane shared = { DM1_VIEW_SQUARE_D4L, PIXEL_OWNER_BACKGROUND, "background" };
    DM1_ViewportCellOrder d4_order;

    check_bool("d4_left.d4l_nonnull", d4l != NULL, true);
    check_bool("d4_left.d3l2_nonnull", d3l2 != NULL, true);
    check_bool("d4_left.d3l_nonnull", d3l != NULL, true);
    if (!d4l || !d3l2 || !d3l) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8466-8469 draw D4L object pixels
     * before the MEDIA720 D3L2 helper at lines 8478-8482 and the D3L wall
     * helper at lines 8488-8491.  DUNVIEW.C F0116 lines 6406-6437 then
     * draws the D3L wall and returns before a same-square F0115 thing pass
     * unless the front ornament is an alcove. */
    check_bool("d4_left.draw_order_found",
               d4l_order >= 0 && d3l2_order >= 0 && d3l_order >= 0, true);
    check_bool("d4_left.d4_before_d3l2", d4l_order < d3l2_order, true);
    check_bool("d4_left.d3l2_before_d3l", d3l2_order < d3l_order, true);
    check_contains("d4_left.source_d4", d4l->source_lines, "8466-8469");
    check_contains("d4_left.source_d3l2", d3l2->source_lines, "6254-6260");
    check_contains("d4_left.source_d3l", d3l->source_lines, "6421-6427");
    check_contains("d4_left.occlusion_d3l", d3l->occlusion_source_lines, "6432-6437");

    d4_order = dm1_viewport_3d_decode_cell_order(d4l->cell_order);
    check_int("d4_left.cell_order", d4l->cell_order, 0x0001);
    check_int("d4_left.cell_count", d4_order.cell_count, 1);
    check_int("d4_left.cell0", d4_order.cells[0], 1);
    check_bool("d4_left.uses_first_object", d4l->uses_square_first_object, true);

    check_bool("d4_left.d3l2_wall_returns", d3l2->wall_case_returns, true);
    check_bool("d4_left.d3l_wall_returns", d3l->wall_case_returns, true);
    check_bool("d4_left.d3l2_no_front_alcove_reveal",
               d3l2->front_alcove_reveals_contents, false);
    check_bool("d4_left.d3l_front_alcove_reveal_recorded",
               d3l->front_alcove_reveals_contents, true);

    write_pixel(&shared, d4l->square, PIXEL_OWNER_D4_OBJECT, "D4L object F0115");
    check_int("d4_left.after_d4_owner", shared.owner, PIXEL_OWNER_D4_OBJECT);
    write_pixel(&shared, d3l2->square, PIXEL_OWNER_D3L2_SIDE_WALL, "D3L2 wall");
    check_int("d4_left.after_d3l2_owner", shared.owner, PIXEL_OWNER_D3L2_SIDE_WALL);
    write_pixel(&shared, d3l->square, PIXEL_OWNER_D3L_WALL, "D3L wall");
    check_int("d4_left.final_owner", shared.owner, PIXEL_OWNER_D3L_WALL);
    check_int("d4_left.final_square", shared.square, DM1_VIEW_SQUARE_D3L);
}

static void verify_far_right_object_hidden_by_d3_side_walls(void)
{
    const DM1_ViewportFarObjectPassSpec *d4r =
        dm1_viewport_3d_get_far_object_pass_spec_for_square(DM1_VIEW_SQUARE_D4R);
    const DM1_ViewportWallDrawSpec *d3r2 =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3R2);
    const DM1_ViewportWallDrawSpec *d3r =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3R);
    const int d4r_order = draw_order_index(DM1_VIEW_SQUARE_D4R);
    const int d3r2_order = draw_order_index(DM1_VIEW_SQUARE_D3R2);
    const int d3r_order = draw_order_index(DM1_VIEW_SQUARE_D3R);
    PixelLane shared = { DM1_VIEW_SQUARE_D4R, PIXEL_OWNER_BACKGROUND, "background" };
    DM1_ViewportCellOrder d4_order;

    check_bool("d4_right.d4r_nonnull", d4r != NULL, true);
    check_bool("d4_right.d3r2_nonnull", d3r2 != NULL, true);
    check_bool("d4_right.d3r_nonnull", d3r != NULL, true);
    if (!d4r || !d3r2 || !d3r) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0128 lines 8470-8473 draw D4R object pixels
     * before the MEDIA720 D3R2 helper at lines 8483-8486 and the D3R wall
     * helper at lines 8492-8495.  DUNVIEW.C F0117 lines 6545-6573 then
     * draws the D3R wall and returns before a same-square F0115 thing pass
     * unless the front ornament is an alcove. */
    check_bool("d4_right.draw_order_found",
               d4r_order >= 0 && d3r2_order >= 0 && d3r_order >= 0, true);
    check_bool("d4_right.d4_before_d3r2", d4r_order < d3r2_order, true);
    check_bool("d4_right.d3r2_before_d3r", d3r2_order < d3r_order, true);
    check_contains("d4_right.source_d4", d4r->source_lines, "8470-8473");
    check_contains("d4_right.source_d3r2", d3r2->source_lines, "6321-6327");
    check_contains("d4_right.source_d3r", d3r->source_lines, "6554-6564");
    check_contains("d4_right.occlusion_d3r", d3r->occlusion_source_lines, "6568-6573");

    d4_order = dm1_viewport_3d_decode_cell_order(d4r->cell_order);
    check_int("d4_right.redmcsb_view_square", d4r->redmcsb_view_square_id, 18);
    check_int("d4_right.cell_order", d4r->cell_order, 0x0001);
    check_int("d4_right.cell_count", d4_order.cell_count, 1);
    check_int("d4_right.cell0", d4_order.cells[0], 1);
    check_bool("d4_right.uses_first_object", d4r->uses_square_first_object, true);

    check_bool("d4_right.d3r2_wall_returns", d3r2->wall_case_returns, true);
    check_bool("d4_right.d3r_wall_returns", d3r->wall_case_returns, true);
    check_bool("d4_right.d3r2_no_front_alcove_reveal",
               d3r2->front_alcove_reveals_contents, false);
    check_bool("d4_right.d3r_front_alcove_reveal_recorded",
               d3r->front_alcove_reveals_contents, true);

    write_pixel(&shared, d4r->square, PIXEL_OWNER_D4_OBJECT, "D4R object F0115");
    check_int("d4_right.after_d4_owner", shared.owner, PIXEL_OWNER_D4_OBJECT);
    write_pixel(&shared, d3r2->square, PIXEL_OWNER_D3R2_SIDE_WALL, "D3R2 wall");
    check_int("d4_right.after_d3r2_owner", shared.owner, PIXEL_OWNER_D3R2_SIDE_WALL);
    write_pixel(&shared, d3r->square, PIXEL_OWNER_D3R_WALL, "D3R wall");
    check_int("d4_right.final_owner", shared.owner, PIXEL_OWNER_D3R_WALL);
    check_int("d4_right.final_square", shared.square, DM1_VIEW_SQUARE_D3R);
}

static void verify_far_center_object_hidden_by_d3_center_wall(void)
{
    const DM1_ViewportFarObjectPassSpec *d4c =
        dm1_viewport_3d_get_far_object_pass_spec_for_square(DM1_VIEW_SQUARE_D4C);
    const DM1_ViewportWallDrawSpec *d3c =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D3C);
    const int d4c_order = draw_order_index(DM1_VIEW_SQUARE_D4C);
    const int d3c_order = draw_order_index(DM1_VIEW_SQUARE_D3C);
    PixelLane shared = { DM1_VIEW_SQUARE_D4C, PIXEL_OWNER_BACKGROUND, "background" };

    check_bool("d4_center.d4c_nonnull", d4c != NULL, true);
    check_bool("d4_center.d3c_nonnull", d3c != NULL, true);
    if (!d4c || !d3c) {
        return;
    }

    check_bool("d4_center.draw_order_found", d4c_order >= 0 && d3c_order >= 0, true);
    check_bool("d4_center.d4_before_d3c", d4c_order < d3c_order, true);
    check_int("d4_center.redmcsb_view_square", d4c->redmcsb_view_square_id, 16);
    check_contains("d4_center.source_d4", d4c->source_lines, "8474-8477");
    check_contains("d4_center.source_d3c", d3c->source_lines, "6707-6714");
    check_contains("d4_center.occlusion_d3c", d3c->occlusion_source_lines, "6716-6720");
    check_bool("d4_center.center_wall", d3c->center_wall, true);
    check_bool("d4_center.wall_returns", d3c->wall_case_returns, true);

    write_pixel(&shared, d4c->square, PIXEL_OWNER_D4_OBJECT, "D4C object F0115");
    check_int("d4_center.after_d4_owner", shared.owner, PIXEL_OWNER_D4_OBJECT);
    write_pixel(&shared, d3c->square, PIXEL_OWNER_D3C_WALL, "D3C wall");
    check_int("d4_center.final_owner", shared.owner, PIXEL_OWNER_D3C_WALL);
    check_int("d4_center.final_square", shared.square, DM1_VIEW_SQUARE_D3C);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_d4_far_object_d3_wall_pixel_gate\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceLocks=F0128:8466-8499,F0116:6406-6437,F0118:6697-6720\n");
    printf("claim=synthetic pixel-owner gate for D4 far object occlusion by D3 walls; no screenshot parity claim\n");

    check_int("far_object.spec_count", (int)dm1_viewport_3d_far_object_pass_spec_count(), 3);
    verify_far_left_object_hidden_by_d3_side_walls();
    verify_far_right_object_hidden_by_d3_side_walls();
    verify_far_center_object_hidden_by_d3_center_wall();

    if (failures) {
        fprintf(stderr, "result=fail failures=%d\n", failures);
        return 1;
    }
    printf("result=pass\n");
    return 0;
}
