#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    PIXEL_OWNER_BACKGROUND = 0,
    PIXEL_OWNER_WALL = 1,
    PIXEL_OWNER_ALCOVE_THING = 2
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

static void write_pixel(PixelLane *pixel,
                        DM1_ViewSquareIndex square,
                        PixelOwner owner,
                        const char *phase)
{
    pixel->square = square;
    pixel->owner = owner;
    pixel->phase = phase;
}

static void verify_d2c_front_alcove_reveals_thing_pixels(void)
{
    const DM1_ViewportWallDrawSpec *d2c =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D2C);
    PixelLane solid_wall_pixel = {
        DM1_VIEW_SQUARE_D2C, PIXEL_OWNER_BACKGROUND, "background"
    };
    PixelLane alcove_pixel = {
        DM1_VIEW_SQUARE_D2C, PIXEL_OWNER_BACKGROUND, "background"
    };
    DM1_ViewportCellOrder alcove_order = dm1_viewport_3d_decode_cell_order(0x0000);

    check_bool("d2c_alcove.nonnull", d2c != NULL, true);
    if (!d2c) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0121 lines 7299-7306 draw the D2C front wall
     * first.  Lines 7308-7312 check F0107's front-wall ornament return; a
     * true alcove switches to C0x0000_CELL_ORDER_ALCOVE and continues into
     * the same square F0115 handoff instead of returning from the wall case. */
    check_contains("d2c_alcove.source_wall", d2c->source_lines, "7299-7306");
    check_contains("d2c_alcove.source_alcove", d2c->occlusion_source_lines, "7308-7312");
    check_bool("d2c_alcove.center_wall", d2c->center_wall, true);
    check_bool("d2c_alcove.wall_case_returns", d2c->wall_case_returns, true);
    check_bool("d2c_alcove.front_alcove_reveals", d2c->front_alcove_reveals_contents, true);
    check_int("d2c_alcove.zone", d2c->pc34_zone, DM1_PC34_ZONE_WALL_D2C);

    write_pixel(&solid_wall_pixel, d2c->square, PIXEL_OWNER_WALL, "D2C solid wall");
    if (!dm1_viewport_3d_wall_occludes_floor_items(d2c, false)) {
        write_pixel(&solid_wall_pixel, d2c->square, PIXEL_OWNER_ALCOVE_THING, "unexpected thing");
    }
    check_int("d2c_solid.final_owner", solid_wall_pixel.owner, PIXEL_OWNER_WALL);
    check_int("d2c_solid.cell_order", dm1_viewport_3d_wall_item_cell_order(d2c, false), 0xffff);
    check_bool("d2c_solid.projectile_hidden",
               dm1_viewport_3d_projectile_visible_after_wall_case(d2c, false), false);

    write_pixel(&alcove_pixel, d2c->square, PIXEL_OWNER_WALL, "D2C wall shell");
    if (!dm1_viewport_3d_wall_occludes_floor_items(d2c, true)) {
        write_pixel(&alcove_pixel, d2c->square, PIXEL_OWNER_ALCOVE_THING, "D2C alcove F0115");
    }
    check_int("d2c_alcove.final_owner", alcove_pixel.owner, PIXEL_OWNER_ALCOVE_THING);
    check_int("d2c_alcove.final_square", alcove_pixel.square, DM1_VIEW_SQUARE_D2C);
    check_int("d2c_alcove.cell_order", dm1_viewport_3d_wall_item_cell_order(d2c, true), 0x0000);
    check_bool("d2c_alcove.cell_order_decodes_alcove", alcove_order.alcove, true);
    check_int("d2c_alcove.cell_count", alcove_order.cell_count, 0);
    check_bool("d2c_alcove.projectile_visible",
               dm1_viewport_3d_projectile_visible_after_wall_case(d2c, true), true);
}

static void verify_d1l_side_wall_still_occludes_even_with_front_alcove_flag(void)
{
    const DM1_ViewportWallDrawSpec *d1l =
        dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_VIEW_SQUARE_D1L);
    PixelLane pixel = {
        DM1_VIEW_SQUARE_D1L, PIXEL_OWNER_BACKGROUND, "background"
    };

    check_bool("d1l_negative.nonnull", d1l != NULL, true);
    if (!d1l) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0122 lines 7445-7460 draw the D1L side wall
     * and side ornament, then return.  Unlike the D2C front-wall alcove
     * branch, there is no C0x0000 alcove handoff for this side-wall slice. */
    check_contains("d1l_negative.source_wall", d1l->source_lines, "7445-7455");
    check_contains("d1l_negative.source_return", d1l->occlusion_source_lines, "7459-7460");
    check_bool("d1l_negative.front_alcove_reveals", d1l->front_alcove_reveals_contents, false);

    write_pixel(&pixel, d1l->square, PIXEL_OWNER_WALL, "D1L side wall");
    if (!dm1_viewport_3d_wall_occludes_floor_items(d1l, true)) {
        write_pixel(&pixel, d1l->square, PIXEL_OWNER_ALCOVE_THING, "unexpected thing");
    }
    check_int("d1l_negative.final_owner", pixel.owner, PIXEL_OWNER_WALL);
    check_int("d1l_negative.cell_order", dm1_viewport_3d_wall_item_cell_order(d1l, true), 0xffff);
    check_bool("d1l_negative.projectile_hidden",
               dm1_viewport_3d_projectile_visible_after_wall_case(d1l, true), false);
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_d2c_front_alcove_pixel_gate\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceLocks=F0121:7299-7312,F0122:7445-7460\n");
    printf("claim=synthetic pixel-owner gate for D2C front-alcove reveal; no screenshot parity claim\n");

    verify_d2c_front_alcove_reveals_thing_pixels();
    verify_d1l_side_wall_still_occludes_even_with_front_alcove_flag();

    if (failures) {
        fprintf(stderr, "result=fail failures=%d\n", failures);
        return 1;
    }
    printf("result=pass\n");
    return 0;
}
