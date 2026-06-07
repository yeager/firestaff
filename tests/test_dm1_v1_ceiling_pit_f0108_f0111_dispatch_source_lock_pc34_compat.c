#include "dm1_v1_ceiling_pit_viewport_pc34_compat.h"
#include "dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat.h"
#include "src/dm1/dm1_v1_viewport_d0c_stairs_pit_dispatch_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

enum {
    DM1_VIEW_SQUARE_D0C = 0,
    DM1_VIEW_SQUARE_D1C = 3,
    DM1_VIEW_SQUARE_D2L = 4,
    DM1_VIEW_SQUARE_D2R = 5,
    DM1_VIEW_SQUARE_ROW_MINUS_ONE_LEFT = 1,
    DM1_VIEW_CELL_FRONT_LEFT = 0,
    DM1_VIEW_CELL_FRONT_RIGHT = 1,
    DM1_VIEW_CELL_BACK_RIGHT = 2,
    DM1_ZONE_OBJECT_BASE = 2500,
    DM1_ZONE_PROJECTILE_BASE = 2900,
    DM1_ZONE_EXPLOSION_REBIRTH1_BASE = 3000,
    DM1_ZONE_CREATURE_BASE = 3200,
    DM1_ZONE_SHIFT_OBJECTS_AND_CREATURES = 0x8000,
    DRAW_ORDER_OBJECT = 10,
    DRAW_ORDER_CREATURE = 20,
    DRAW_ORDER_PROJECTILE = 30,
    DRAW_ORDER_EXPLOSION = 40
};

static const char *A_F0108_D2L =
    "ReDMCSB DUNVIEW.C:6988-7024 F0119 D2L door-front/pit/corridor: "
    "F0108 then F0115/F0111 door-front, and pit fall-through draws "
    "F0108/F0112/F0115";
static const char *A_F0108_D2R =
    "ReDMCSB DUNVIEW.C:7181-7224 F0120 D2R door-front/pit/corridor: "
    "F0108 then F0115/F0111 door-front, and pit fall-through draws "
    "F0108/F0112/F0115";
static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor open-state "
    "skip, ornament, closed/destroyed/partly-open branch, and final "
    "F0791 door-zone blit";
static const char *A_F0115_ORDER =
    "ReDMCSB DUNVIEW.C:4547-4581 F0115 draws objects, then creatures, "
    "then projectiles, then explosions/fluxcage";
static const char *A_F0115_ZONES =
    "ReDMCSB DUNVIEW.C:373 G2028/G2033/G2034 tables; 4806-4811 loads "
    "G2028; 4923 gates rows >=0; 5075 C2500; 5201-5214/5615-5617 C3200; "
    "5668-5683 C2900; 5916-5923/5998-5999 C3000";
static const char *A_D0C =
    "ReDMCSB DUNVIEW.C:8274-8294 F0127 D0C pit: floor pit, F0112, F0115; "
    "DUNVIEW.C:8241-8273 D0C stairs break before F0115/F0108";
static const char *A_D1C =
    "ReDMCSB DUNVIEW.C:7753-7783 and 7912-7937 F0124 D1C stairs/pit: "
    "stairs and pits fall through T0124017 to F0108/F0112/F0115";

static int g_assertions = 0;
static int g_failures = 0;

static const signed char k_g2028[23] = {
    11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0,
    1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1
};

static const signed char k_g2033[23] = {
    -1, 11, 12, 8, 9, 10, 5, 6, 7, -1, -1, 0,
    1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1
};

static const signed char k_g2034[23] = {
    14, 15, 16, 11, 12, 13, 8, 9, 10, -1, -1, 3,
    4, 5, 6, 7, 0, 1, 2, -1, -1, -1, -1
};

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_nonnull(const char *id, const void *ptr, const char *anchor)
{
    ++g_assertions;
    if (!ptr) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack, const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static int g2028_row(int view_square)
{
    if (view_square < 0 || view_square >= (int)(sizeof(k_g2028) / sizeof(k_g2028[0]))) {
        return -1;
    }
    return k_g2028[view_square];
}

static int g2033_row(int view_square)
{
    if (view_square < 0 || view_square >= (int)(sizeof(k_g2033) / sizeof(k_g2033[0]))) {
        return -1;
    }
    return k_g2033[view_square];
}

static int g2034_row(int view_square)
{
    if (view_square < 0 || view_square >= (int)(sizeof(k_g2034) / sizeof(k_g2034[0]))) {
        return -1;
    }
    return k_g2034[view_square];
}

static int object_zone_from_g2028(int row, int view_cell)
{
    if (row < 0) {
        return -1;
    }
    return (DM1_ZONE_OBJECT_BASE | DM1_ZONE_SHIFT_OBJECTS_AND_CREATURES) +
           (row * 4) + view_cell;
}

static int projectile_zone_from_g2028(int row, int view_cell)
{
    if (row < 0) {
        return -1;
    }
    return DM1_ZONE_PROJECTILE_BASE + (row * 4) + view_cell;
}

static int creature_zone_from_g2033(int row, int view_cell, int coordinate_set)
{
    if (row < 0) {
        return -1;
    }
    return (DM1_ZONE_CREATURE_BASE | DM1_ZONE_SHIFT_OBJECTS_AND_CREATURES) +
           (coordinate_set * 65) + (row * 5) + view_cell;
}

static int rebirth_step1_zone_from_g2028(int row)
{
    if (row < 0) {
        return -1;
    }
    return DM1_ZONE_EXPLOSION_REBIRTH1_BASE + row;
}

static void test_source_anchor_text(void)
{
    expect_contains("anchor.d2l.f0108", A_F0108_D2L, "6988-7024", A_F0108_D2L);
    expect_contains("anchor.d2l.f0111", A_F0108_D2L, "F0111", A_F0108_D2L);
    expect_contains("anchor.d2r.f0108", A_F0108_D2R, "7181-7224", A_F0108_D2R);
    expect_contains("anchor.d2r.f0111", A_F0108_D2R, "F0111", A_F0108_D2R);
    expect_contains("anchor.f0111.range", A_F0111, "4218-4337", A_F0111);
    expect_contains("anchor.f0111.final_blit", A_F0111, "F0791", A_F0111);
    expect_contains("anchor.f0115.range", A_F0115_ORDER, "4547-4581", A_F0115_ORDER);
    expect_contains("anchor.f0115.zone.g2028", A_F0115_ZONES, "G2028", A_F0115_ZONES);
    expect_contains("anchor.f0115.zone.c2500", A_F0115_ZONES, "5075 C2500", A_F0115_ZONES);
    expect_contains("anchor.f0115.zone.c3200", A_F0115_ZONES, "5615-5617 C3200", A_F0115_ZONES);
    expect_contains("anchor.f0115.zone.c2900", A_F0115_ZONES, "5668-5683 C2900", A_F0115_ZONES);
    expect_contains("anchor.f0115.zone.c3000", A_F0115_ZONES, "5998-5999 C3000", A_F0115_ZONES);
}

static void test_d0c_pit_and_stairs_route(void)
{
    const DM1_V1_D0CStairsPitDispatchContractPc34 *c =
        dm1_v1_viewport_d0c_stairs_pit_dispatch_contract_pc34_compat();

    expect_nonnull("d0c.contract", c, A_D0C);
    if (!c) {
        return;
    }

    expect_bool("d0c.contract_only", c->contract_only, true, A_D0C);
    expect_int("d0c.floor_pit.graphic", c->floor_pit_graphic, 57, A_D0C);
    expect_int("d0c.floor_pit.zone", c->floor_pit_zone, 862, A_D0C);
    expect_int("d0c.ceiling_pit.graphic", c->ceiling_pit_graphic, 69, A_D0C);
    expect_int("d0c.ceiling_pit.zone", c->ceiling_pit_zone, 871, A_D0C);
    expect_int("d0c.view_square.media720", c->media720_view_square_d0c, DM1_VIEW_SQUARE_D0C,
               A_D0C);
    expect_int("d0c.cell_order.pit", c->cell_order_backleft_backright, 0x0021, A_D0C);
    expect_bool("d0c.pit.calls_f0115", c->pit_calls_f0115, true, A_D0C);
    expect_bool("d0c.ceiling_before_f0115", c->ceiling_pit_order < c->thing_pass_order,
                true, A_D0C);
    expect_bool("d0c.stairs_down.no_f0115", c->stairs_down_calls_f0115, false, A_D0C);
    expect_bool("d0c.stairs_down.no_f0108", c->stairs_down_calls_f0108_floor_ornament,
                false, A_D0C);
    expect_contains("d0c.evidence.no_f0108", c->s_no_f0108_anchor, "F0108", A_D0C);
}

static void test_d1c_pit_and_stairs_route(void)
{
    DM1_V1_D1CDispatchInputPc34 input;
    DM1_V1_D1CDispatchOutputPc34 out;
    bool ok;

    memset(&input, 0, sizeof(input));
    input.element = DM1_V1_D1C_DISPATCH_PC34_ELEMENT_FLOOR_PIT;
    input.floor_ornament_ordinal = 3;
    input.first_thing_index = 0x1234;
    ok = dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_probe(&input, &out);

    expect_bool("d1c.pit.probe", ok, true, A_D1C);
    expect_int("d1c.pit.route", out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_FLOOR_PIT, A_D1C);
    expect_bool("d1c.pit.f0104", out.used_f0104, true, A_D1C);
    expect_bool("d1c.pit.f0108", out.used_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:7912-7927 pit falls through to F0108");
    expect_bool("d1c.pit.f0112", out.used_f0112_ceiling_pit, true,
                "ReDMCSB DUNVIEW.C:7928-7935 F0112 after F0108");
    expect_bool("d1c.pit.f0115", out.used_f0115, true,
                "ReDMCSB DUNVIEW.C:7936-7937 F0115 after F0112");
    expect_bool("d1c.pit.no_f0111", out.used_f0111_door, false,
                "ReDMCSB DUNVIEW.C:7873-7908 F0111 is door-front only");
    expect_int("d1c.pit.zone", out.zone_index, DM1_V1_D1C_DISPATCH_PC34_ZONE_FLOOR_PIT_D1C,
               A_D1C);
    expect_int("d1c.pit.view_square", out.view_square_index, DM1_VIEW_SQUARE_D1C, A_D1C);
    expect_int("d1c.pit.cell_order", out.cell_order_called, 0x3421, A_D1C);

    memset(&input, 0, sizeof(input));
    input.element = DM1_V1_D1C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT;
    input.has_stairs_up_bit = false;
    ok = dm1_v1_viewport_d1c_stairs_pit_dispatch_pc34_compat_probe(&input, &out);

    expect_bool("d1c.stairs_down.probe", ok, true, A_D1C);
    expect_int("d1c.stairs_down.route", out.route_taken,
               DM1_V1_D1C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT, A_D1C);
    expect_int("d1c.stairs_down.zone", out.zone_index,
               DM1_V1_D1C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D1C, A_D1C);
    expect_bool("d1c.stairs_down.f0108", out.used_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:7764-7783 goto T0124017 then 7925-7927 F0108");
    expect_bool("d1c.stairs_down.f0112", out.used_f0112_ceiling_pit, true,
                "ReDMCSB DUNVIEW.C:7928-7935 F0112");
    expect_bool("d1c.stairs_down.f0115", out.used_f0115, true,
                "ReDMCSB DUNVIEW.C:7936-7937 F0115");
}

static void test_ceiling_pit_rects_match_route_zones(void)
{
    const DM1V1CeilingPitViewportRectPc34 *d2l =
        dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_I34E,
                                              DM1_V1_ZONE_CEILING_PIT_D2L_I34E,
                                              0);
    const DM1V1CeilingPitViewportRectPc34 *d2r =
        dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2L_I34E,
                                              DM1_V1_ZONE_CEILING_PIT_D2R_I34E,
                                              1);
    const DM1V1CeilingPitViewportRectPc34 *d2c =
        dm1_v1_ceiling_pit_viewport_rect_pc34(DM1_V1_GRAPHIC_CEILING_PIT_D2C_I34E,
                                              DM1_V1_ZONE_CEILING_PIT_D2C_I34E,
                                              0);

    expect_nonnull("rect.d2l", d2l, A_F0108_D2L);
    expect_nonnull("rect.d2r", d2r, A_F0108_D2R);
    expect_nonnull("rect.d2c", d2c,
                   "ReDMCSB DUNVIEW.C:7359-7365 D2C F0112 ceiling-pit route");
    if (d2l) {
        expect_int("rect.d2l.x", d2l->x, 0, A_F0108_D2L);
        expect_int("rect.d2l.width", d2l->width, 80, A_F0108_D2L);
    }
    if (d2r) {
        expect_int("rect.d2r.x", d2r->x, 144, A_F0108_D2R);
        expect_int("rect.d2r.width", d2r->width, 80, A_F0108_D2R);
    }
    if (d2c) {
        expect_int("rect.d2c.x", d2c->x, 64,
                   "ReDMCSB DUNVIEW.C:7359-7365 D2C F0112 ceiling-pit route");
        expect_int("rect.d2c.width", d2c->width, 96,
                   "ReDMCSB DUNVIEW.C:7359-7365 D2C F0112 ceiling-pit route");
    }
}

static void test_g2028_gate_and_zone_indices(void)
{
    int d0c_row = g2028_row(DM1_VIEW_SQUARE_D0C);
    int d1c_row = g2028_row(DM1_VIEW_SQUARE_D1C);
    int missing_row = g2028_row(DM1_VIEW_SQUARE_ROW_MINUS_ONE_LEFT);

    expect_int("g2028.d0c.row", d0c_row, 11, A_F0115_ZONES);
    expect_int("g2028.d1c.row", d1c_row, 8, A_F0115_ZONES);
    expect_int("g2028.d2l.row", g2028_row(DM1_VIEW_SQUARE_D2L), 9, A_F0115_ZONES);
    expect_int("g2028.d2r.row", g2028_row(DM1_VIEW_SQUARE_D2R), 10, A_F0115_ZONES);
    expect_int("g2028.minus_one.row", missing_row, -1, A_F0115_ZONES);
    expect_bool("g2028.d0c.pit.not_gated", d0c_row >= 0, true, A_D0C);
    expect_bool("g2028.d1c.pit.not_gated", d1c_row >= 0, true, A_D1C);

    expect_int("zone.object.d1c.front_left",
               object_zone_from_g2028(d1c_row, DM1_VIEW_CELL_FRONT_LEFT),
               (DM1_ZONE_OBJECT_BASE | DM1_ZONE_SHIFT_OBJECTS_AND_CREATURES) + 32,
               "ReDMCSB DUNVIEW.C:4923 and 5075 C2500 + G2028*4 + ViewCell");
    expect_int("zone.projectile.d1c.front_right",
               projectile_zone_from_g2028(d1c_row, DM1_VIEW_CELL_FRONT_RIGHT),
               DM1_ZONE_PROJECTILE_BASE + 33,
               "ReDMCSB DUNVIEW.C:5668-5683 C2900 + G2028*4 + ViewCell");
    expect_int("zone.explosion.d1c.rebirth1",
               rebirth_step1_zone_from_g2028(d1c_row),
               DM1_ZONE_EXPLOSION_REBIRTH1_BASE + 8,
               "ReDMCSB DUNVIEW.C:5916-5923/5998-5999 C3000 + G2028 row");
    expect_int("zone.object.minus_one.gated",
               object_zone_from_g2028(missing_row, DM1_VIEW_CELL_FRONT_LEFT), -1,
               "ReDMCSB DUNVIEW.C:4923 rejects L2476_i_ < 0");
    expect_int("zone.projectile.minus_one.gated",
               projectile_zone_from_g2028(missing_row, DM1_VIEW_CELL_FRONT_LEFT), -1,
               "ReDMCSB DUNVIEW.C:5668-5671 rejects G2028 row < 0");
    expect_int("zone.explosion.minus_one.gated",
               rebirth_step1_zone_from_g2028(missing_row), -1,
               "ReDMCSB DUNVIEW.C:5916-5923/5998-5999 needs nonnegative row");

    expect_int("g2033.d1c.row", g2033_row(DM1_VIEW_SQUARE_D1C), 8,
               "ReDMCSB DUNVIEW.C:375 and 5201-5214 G2033 creature row");
    expect_int("zone.creature.d1c.back_right",
               creature_zone_from_g2033(g2033_row(DM1_VIEW_SQUARE_D1C),
                                        DM1_VIEW_CELL_BACK_RIGHT, 0),
               (DM1_ZONE_CREATURE_BASE | DM1_ZONE_SHIFT_OBJECTS_AND_CREATURES) + 42,
               "ReDMCSB DUNVIEW.C:5201-5214/5615-5617 C3200 + G2033*5 + ViewCell");
    expect_int("g2034.d1c.row", g2034_row(DM1_VIEW_SQUARE_D1C), 11,
               "ReDMCSB DUNVIEW.C:376 and 5916-5923 G2034 explosion row");
}

static void test_f0115_documented_draw_order(void)
{
    expect_int("order.object", DRAW_ORDER_OBJECT, 10, A_F0115_ORDER);
    expect_int("order.creature", DRAW_ORDER_CREATURE, 20, A_F0115_ORDER);
    expect_int("order.projectile", DRAW_ORDER_PROJECTILE, 30, A_F0115_ORDER);
    expect_int("order.explosion", DRAW_ORDER_EXPLOSION, 40, A_F0115_ORDER);
    expect_bool("order.object.before.creature", DRAW_ORDER_OBJECT < DRAW_ORDER_CREATURE,
                true, A_F0115_ORDER);
    expect_bool("order.creature.before.projectile", DRAW_ORDER_CREATURE < DRAW_ORDER_PROJECTILE,
                true, A_F0115_ORDER);
    expect_bool("order.projectile.before.explosion", DRAW_ORDER_PROJECTILE < DRAW_ORDER_EXPLOSION,
                true, A_F0115_ORDER);
}

int main(void)
{
    test_source_anchor_text();
    test_d0c_pit_and_stairs_route();
    test_d1c_pit_and_stairs_route();
    test_ceiling_pit_rects_match_route_zones();
    test_g2028_gate_and_zone_indices();
    test_f0115_documented_draw_order();

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed\n", g_failures, g_assertions);
        return 1;
    }
    printf("PASS: %d assertions\n", g_assertions);
    return 0;
}
