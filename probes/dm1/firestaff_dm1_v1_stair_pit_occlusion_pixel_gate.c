#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    PIXEL_OWNER_BACKGROUND = 0,
    PIXEL_OWNER_D2_STAIR_OR_PIT,
    PIXEL_OWNER_D2_FLOOR_ORNAMENT,
    PIXEL_OWNER_D2_THING_LAYER,
    PIXEL_OWNER_D1_STAIR_OR_PIT,
    PIXEL_OWNER_D1_FLOOR_ORNAMENT,
    PIXEL_OWNER_D1_THING_LAYER
} PixelOwner;

typedef struct {
    DM1_ViewSquareIndex square;
    PixelOwner owner;
    const char *phase;
} Pixel;

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
    check_int(id, text && strstr(text, needle) ? 1 : 0, 1);
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

static void write_pixel(Pixel *pixel,
                        const DM1_ViewportFloorFieldOrderSpec *spec,
                        PixelOwner owner,
                        const char *phase)
{
    pixel->square = spec->square;
    pixel->owner = owner;
    pixel->phase = phase;
}

static void simulate_floor_field_pixel(Pixel *pixel,
                                       const DM1_ViewportFloorFieldOrderSpec *spec,
                                       bool use_stairs,
                                       PixelOwner floor_owner,
                                       PixelOwner ornament_owner,
                                       PixelOwner thing_owner)
{
    if (use_stairs) {
        check_bool("floor_field.stairs_enabled", spec->stairs_draw_before_floor_ornament, true);
        write_pixel(pixel, spec, floor_owner, "stairs");
    } else {
        check_bool("floor_field.pit_enabled", spec->pit_draw_before_floor_ornament, true);
        write_pixel(pixel, spec, floor_owner, "pit");
    }

    if (spec->floor_ornament_before_things) {
        write_pixel(pixel, spec, ornament_owner, "floor_ornament");
    }
    if (spec->objects_creatures_projectiles_before_explosions) {
        write_pixel(pixel, spec, thing_owner, "thing_layer");
    }
}

static void verify_d2_open_pit_can_be_covered_by_floor_ornament(void)
{
    const DM1_ViewportFloorFieldOrderSpec *d2c =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D2C);
    Pixel pixel = { DM1_VIEW_SQUARE_D4C, PIXEL_OWNER_BACKGROUND, "background" };

    check_bool("d2c.nonnull", d2c != NULL, true);
    if (!d2c) {
        return;
    }

    /* ReDMCSB: DUNVIEW.C F0121 lines 7343-7357 draw D2C open-pit pixels,
     * then intentionally allow F0108 floor-ornament pixels to overdraw them
     * on the BUG0_64 path before the F0115 thing pass at lines 7367-7368. */
    check_contains("d2c.pit_source", d2c->pit_source_lines, "BUG0_64");
    check_contains("d2c.floor_source", d2c->floor_ornament_source_lines, "F0108");
    write_pixel(&pixel, d2c, PIXEL_OWNER_D2_STAIR_OR_PIT, "pit");
    check_int("d2c.open_pit.first_owner", pixel.owner, PIXEL_OWNER_D2_STAIR_OR_PIT);

    if (d2c->floor_ornament_before_things) {
        write_pixel(&pixel, d2c, PIXEL_OWNER_D2_FLOOR_ORNAMENT, "floor_ornament");
    }
    check_int("d2c.open_pit.floor_ornament_occludes", pixel.owner,
              PIXEL_OWNER_D2_FLOOR_ORNAMENT);
    check_int("d2c.open_pit.same_square", pixel.square, DM1_VIEW_SQUARE_D2C);

    if (d2c->objects_creatures_projectiles_before_explosions) {
        write_pixel(&pixel, d2c, PIXEL_OWNER_D2_THING_LAYER, "thing_layer");
    }
    check_int("d2c.open_pit.thing_layer_can_own_later_pixel", pixel.owner,
              PIXEL_OWNER_D2_THING_LAYER);
    check_contains("d2c.things_source", d2c->things_source_lines, "F0115");
}

static void verify_d1_stair_pit_owns_shared_lane_after_d2(void)
{
    const DM1_ViewportFloorFieldOrderSpec *d2c =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D2C);
    const DM1_ViewportFloorFieldOrderSpec *d1l =
        dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_VIEW_SQUARE_D1L);
    Pixel pixel = { DM1_VIEW_SQUARE_D4C, PIXEL_OWNER_BACKGROUND, "background" };
    int d2c_order;
    int d1l_order;

    check_bool("d1_lane.d2c_nonnull", d2c != NULL, true);
    check_bool("d1_lane.d1l_nonnull", d1l != NULL, true);
    if (!d2c || !d1l) {
        return;
    }

    d2c_order = draw_order_index(DM1_VIEW_SQUARE_D2C);
    d1l_order = draw_order_index(DM1_VIEW_SQUARE_D1L);

    /* ReDMCSB: DUNVIEW.C F0128 lines 8518-8525 call D2C before D1L, so a
     * shared viewport lane touched by both routes is owned by the nearer D1
     * stair/pit/floor/thing sequence, not by the earlier D2 center bitmap. */
    check_bool("d1_lane.draw_order_found", d2c_order >= 0 && d1l_order >= 0, true);
    check_bool("d1_lane.d2_before_d1", d2c_order < d1l_order, true);
    check_contains("d1_lane.d2_source", d2c->stairs_source_lines, "DUNVIEW.C:7260");
    check_contains("d1_lane.d1_source", d1l->stairs_source_lines, "DUNVIEW.C:7405");

    simulate_floor_field_pixel(&pixel,
                               d2c,
                               true,
                               PIXEL_OWNER_D2_STAIR_OR_PIT,
                               PIXEL_OWNER_D2_FLOOR_ORNAMENT,
                               PIXEL_OWNER_D2_THING_LAYER);
    check_int("d1_lane.after_d2_owner", pixel.owner, PIXEL_OWNER_D2_THING_LAYER);
    check_int("d1_lane.after_d2_square", pixel.square, DM1_VIEW_SQUARE_D2C);

    simulate_floor_field_pixel(&pixel,
                               d1l,
                               false,
                               PIXEL_OWNER_D1_STAIR_OR_PIT,
                               PIXEL_OWNER_D1_FLOOR_ORNAMENT,
                               PIXEL_OWNER_D1_THING_LAYER);
    check_int("d1_lane.final_owner", pixel.owner, PIXEL_OWNER_D1_THING_LAYER);
    check_int("d1_lane.final_square", pixel.square, DM1_VIEW_SQUARE_D1L);
    check_contains("d1_lane.d1_pit_source", d1l->pit_source_lines, "DUNVIEW.C:7510");
    check_contains("d1_lane.d1_things_source", d1l->things_source_lines, "F0115");
}

int main(void)
{
    printf("probe=firestaff_dm1_v1_stair_pit_occlusion_pixel_gate\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceLocks=F0121:7260-7368,F0122:7405-7536,F0128:8518-8525\n");
    printf("claim=synthetic pixel-owner gate using source-locked viewport metadata; no screenshot parity claim\n");

    verify_d2_open_pit_can_be_covered_by_floor_ornament();
    verify_d1_stair_pit_owns_shared_lane_after_d2();

    if (failures) {
        fprintf(stderr, "result=fail failures=%d\n", failures);
        return 1;
    }
    printf("result=pass\n");
    return 0;
}
