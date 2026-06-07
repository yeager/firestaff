#include "dm1_v1_viewport_d1c_center_field_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

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

static void expect_nonnull(const char *id, const void *got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        printf("FAIL %s got=NULL at %s\n", id, anchor);
        ++g_failures;
    } else {
        printf("PASS %s nonnull (%s)\n", id, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static uint8_t square_for_type(int type)
{
    return (uint8_t)((type & 0x07) << 5);
}

static DM1_V1_D1CCenterFieldRenderPc34 render_fixture(
    int square_type,
    bool has_item,
    bool has_creature,
    bool has_projectile,
    bool has_explosion,
    bool door_is_open,
    uint8_t *target_pixels,
    uint8_t *viewport_pixels)
{
    static uint8_t cells[9];
    DM1_Viewport3DState viewport;
    DM1_V1_D1CCenterFieldDungeonPc34 dungeon;
    DM1_V1_D1CCenterFieldPartyPc34 party;
    DM1_V1_D1CCenterFieldTargetPc34 target;

    memset(cells, 0, sizeof(cells));
    cells[1] = square_for_type(square_type);

    dm1_viewport_3d_init(&viewport, viewport_pixels, DM1_VIEWPORT_WIDTH);

    dungeon.cells = cells;
    dungeon.width = 3;
    dungeon.height = 3;
    dungeon.has_item = has_item;
    dungeon.has_creature = has_creature;
    dungeon.has_projectile = has_projectile;
    dungeon.has_explosion = has_explosion;
    dungeon.door_is_open = door_is_open;

    party.map_x = 1;
    party.map_y = 1;
    party.direction = 0;

    target.pixels = target_pixels;
    target.width = 16;
    target.height = 16;
    target.stride = 16;
    target.zone = DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ZONE;

    return dm1_v1_viewport_d1c_center_field_pc34_compat_render_square(
        &dungeon, &party, &viewport, &target);
}

static int verify_d1c_relative_square_path(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;
    int16_t x = -1;
    int16_t y = -1;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0x55, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
                         false, false, false, false, false,
                         target, viewport_pixels);

    expect_bool("d1c.called_f0152", out.called_relative_square, true,
                "ReDMCSB DUNGEON.C:1481-1492 F0152");
    expect_bool("d1c.called_f0151", out.called_get_square, true,
                "ReDMCSB DUNGEON.C:1423-1479 F0151");
    expect_bool("d1c.called_f0153", out.called_relative_square_type, true,
                "ReDMCSB DUNGEON.C:1495-1506 F0153");
    expect_int("d1c.target_x", out.target_map_x, 1,
               "ReDMCSB DUNGEON.C:1481-1492 F0152 depth 1 lane 0");
    expect_int("d1c.target_y", out.target_map_y, 0,
               "ReDMCSB DUNGEON.C:1481-1492 F0152 depth 1 lane 0");
    expect_int("d1c.square_type", out.square_type,
               DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
               "ReDMCSB DEFS.H:1001 M034_SQUARE_TYPE");
    dm1_viewport_3d_resolve_relative_map_xy(1, 1, 0, 1, 1, &x, &y);
    expect_int("d1c.relative_east_x", x, 2,
               "ReDMCSB DUNGEON.C:1481-1492 F0152");
    expect_int("d1c.relative_east_y", y, 1,
               "ReDMCSB DUNGEON.C:1481-1492 F0152");
    return g_failures == before;
}

static int verify_empty_center_field(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0x55, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
                         false, false, false, false, false,
                         target, viewport_pixels);

    expect_int("d1c.empty.route", (int)out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_FIELD,
               "ReDMCSB DUNVIEW.C:7922-7937 F0124 no-wall route");
    expect_bool("d1c.empty.floor_ceiling", out.called_floor_ceiling_helper, true,
                "ReDMCSB DUNVIEW.C:2962 F0098");
    expect_bool("d1c.empty.f0115", out.called_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7937 F0115");
    expect_int("d1c.empty.cell_order", (int)out.cell_order, 0x3421,
               "ReDMCSB DUNVIEW.C:7925 C0x3421");
    expect_bool("d1c.empty.no_wall_bitmap", out.drew_wall_bitmap, false,
                "ReDMCSB DUNVIEW.C:7784-7872 excluded");
    expect_bool("d1c.empty.no_wall_ornament", out.drew_wall_ornament, false,
                "ReDMCSB DUNVIEW.C:7842-7843 excluded");
    expect_bool("d1c.empty.no_door_bitmap", out.drew_door_bitmap, false,
                "ReDMCSB DUNVIEW.C:7873-7911 excluded");
    expect_int("d1c.empty.ceiling_marker", target[0], 1,
               "ReDMCSB DUNVIEW.C:2962 F0098");
    expect_int("d1c.empty.floor_marker", target[15 * 16 + 8], 2,
               "ReDMCSB DUNVIEW.C:2962 F0098");
    expect_int("d1c.empty.viewport_cleared", viewport_pixels[0], 0,
               "ReDMCSB DUNVIEW.C:2962 F0098");
    return g_failures == before;
}

static int verify_item_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
                         true, false, false, false, false,
                         target, viewport_pixels);

    expect_bool("d1c.item.drawn", out.drew_item, true,
                "ReDMCSB DUNVIEW.C:4567-4571; DUNVIEW.C:7937 F0115");
    expect_bool("d1c.item.creature_not_drawn", out.drew_creature, false,
                "ReDMCSB DUNVIEW.C:5195-5202; DUNVIEW.C:7937 F0115");
    expect_int("d1c.item.marker", target[4 * 16 + 4], 3,
               "ReDMCSB DUNVIEW.C:4820-4860 F0115 object pass");
    return g_failures == before;
}

static int verify_creature_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
                         false, true, false, false, false,
                         target, viewport_pixels);

    expect_bool("d1c.creature.drawn", out.drew_creature, true,
                "ReDMCSB DUNVIEW.C:5195-5202; DUNVIEW.C:7937 F0115");
    expect_int("d1c.creature.marker", target[4 * 16 + 5], 4,
               "ReDMCSB DUNVIEW.C:5195-5202 F0115 creature pass");
    return g_failures == before;
}

static int verify_projectile_and_explosion_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR,
                         false, false, true, true, false,
                         target, viewport_pixels);

    expect_bool("d1c.projectile.drawn", out.drew_projectile, true,
                "ReDMCSB DUNVIEW.C:5681-5883; DUNVIEW.C:7937 F0115");
    expect_bool("d1c.explosion.drawn", out.drew_explosion, true,
                "ReDMCSB DUNVIEW.C:5915-5933; DUNVIEW.C:7937 F0115");
    expect_int("d1c.projectile.marker", target[4 * 16 + 6], 5,
               "ReDMCSB DUNVIEW.C:5681-5883 F0115 projectile pass");
    expect_int("d1c.explosion.marker", target[4 * 16 + 7], 6,
               "ReDMCSB DUNVIEW.C:5915-5933 F0115 explosion pass");
    return g_failures == before;
}

static int verify_teleporter_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER,
                         true, false, false, false, false,
                         target, viewport_pixels);

    expect_int("d1c.teleporter.route", (int)out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_TELEPORTER_FIELD,
               "ReDMCSB DUNVIEW.C:7942-7956 F0113");
    expect_bool("d1c.teleporter.f0113", out.called_f0113_field, true,
                "ReDMCSB DUNVIEW.C:7955 F0113");
    expect_int("d1c.teleporter.field_zone", out.field_zone, 712,
               "ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C");
    expect_int("d1c.teleporter.field_aspect", out.field_aspect, 10,
               "ReDMCSB DUNVIEW.C:370-377 G2035[3]");
    expect_int("d1c.teleporter.marker", target[8 * 16 + 8], 7,
               "ReDMCSB DUNVIEW.C:7955 F0113");
    expect_bool("d1c.teleporter.no_wall", out.drew_wall_bitmap, false,
                "ReDMCSB DUNVIEW.C:7784-7872 excluded");
    return g_failures == before;
}

static int verify_pit_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_PIT,
                         false, false, false, false, false,
                         target, viewport_pixels);

    expect_int("d1c.pit.route", (int)out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_PIT_FIELD,
               "ReDMCSB DUNVIEW.C:7912-7937");
    expect_bool("d1c.pit.f0104", out.called_f0104_pit_or_stairs, true,
                "ReDMCSB DUNVIEW.C:7912-7920 F0104");
    expect_bool("d1c.pit.f0108", out.called_f0108_floor_ornament, true,
                "ReDMCSB DUNVIEW.C:7926 F0108");
    expect_bool("d1c.pit.f0112", out.called_f0112_ceiling_pit, true,
                "ReDMCSB DUNVIEW.C:7928-7935 F0112");
    expect_int("d1c.pit.marker", target[14 * 16 + 8], 8,
               "ReDMCSB DUNVIEW.C:7912-7920 F0104");
    return g_failures == before;
}

static int verify_stairs_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_STAIRS,
                         false, false, false, false, false,
                         target, viewport_pixels);

    expect_int("d1c.stairs.route", (int)out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_STAIRS_FIELD,
               "ReDMCSB DUNVIEW.C:7751-7783; DUNVIEW.C:7924-7937");
    expect_bool("d1c.stairs.f0104", out.called_f0104_pit_or_stairs, true,
                "ReDMCSB DUNVIEW.C:7754-7780 F0104");
    expect_bool("d1c.stairs.f0115", out.called_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7937 F0115");
    expect_int("d1c.stairs.marker", target[1 * 16 + 8], 9,
               "ReDMCSB DUNVIEW.C:7754-7780 F0104");
    return g_failures == before;
}

static int verify_open_door_square(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 open_out;
    DM1_V1_D1CCenterFieldRenderPc34 closed_out;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    open_out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR,
                              true, true, false, false, true,
                              target, viewport_pixels);
    closed_out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_DOOR,
                                true, true, false, false, false,
                                target, viewport_pixels);

    expect_int("d1c.open_door.route", (int)open_out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_OPEN_DOOR_FIELD,
               "ReDMCSB DUNVIEW.C:7922-7937 no-wall handoff");
    expect_bool("d1c.open_door.f0115", open_out.called_f0115_thing_pass, true,
                "ReDMCSB DUNVIEW.C:7937 F0115");
    expect_bool("d1c.open_door.no_f0111", open_out.drew_door_bitmap, false,
                "ReDMCSB DUNVIEW.C:7873-7911 excluded");
    expect_bool("d1c.open_door.item", open_out.drew_item, true,
                "ReDMCSB DUNVIEW.C:4567-4571 F0115");
    expect_bool("d1c.open_door.creature", open_out.drew_creature, true,
                "ReDMCSB DUNVIEW.C:5195-5202 F0115");
    expect_int("d1c.closed_door.route", (int)closed_out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_CLOSED_DOOR_BLOCKED,
               "ReDMCSB DUNVIEW.C:7873-7911 excluded from no-wall slice");
    expect_bool("d1c.closed_door.no_f0115", closed_out.called_f0115_thing_pass, false,
                "ReDMCSB DUNVIEW.C:7873-7911 excluded");
    return g_failures == before;
}

static int verify_out_of_bounds_wall_fallback(void)
{
    int before = g_failures;
    uint8_t cells[1];
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_Viewport3DState viewport;
    DM1_V1_D1CCenterFieldDungeonPc34 dungeon;
    DM1_V1_D1CCenterFieldPartyPc34 party;
    DM1_V1_D1CCenterFieldTargetPc34 target_desc;
    DM1_V1_D1CCenterFieldRenderPc34 out;

    cells[0] = square_for_type(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_CORRIDOR);
    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    dm1_viewport_3d_init(&viewport, viewport_pixels, DM1_VIEWPORT_WIDTH);

    dungeon.cells = cells;
    dungeon.width = 1;
    dungeon.height = 1;
    dungeon.has_item = true;
    dungeon.has_creature = true;
    dungeon.has_projectile = false;
    dungeon.has_explosion = false;
    dungeon.door_is_open = false;

    party.map_x = 0;
    party.map_y = 0;
    party.direction = 0;

    target_desc.pixels = target;
    target_desc.width = 16;
    target_desc.height = 16;
    target_desc.stride = 16;
    target_desc.zone = DM1_V1_D1C_CENTER_FIELD_PC34_FIELD_ZONE;

    out = dm1_v1_viewport_d1c_center_field_pc34_compat_render_square(
        &dungeon, &party, &viewport, &target_desc);

    expect_int("d1c.oob.route", (int)out.route,
               DM1_V1_D1C_CENTER_FIELD_PC34_ROUTE_WALL_BLOCKED,
               "ReDMCSB DUNGEON.C:1445-1475 F0151 wall fallback");
    expect_int("d1c.oob.square_type", out.square_type,
               DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_WALL,
               "ReDMCSB DUNGEON.C:1445-1475 F0151 wall fallback");
    expect_bool("d1c.oob.no_wall_bitmap", out.drew_wall_bitmap, false,
                "ReDMCSB DUNVIEW.C:7784-7872 excluded");
    expect_bool("d1c.oob.no_wall_ornament", out.drew_wall_ornament, false,
                "ReDMCSB DUNVIEW.C:7842-7843 excluded");
    expect_bool("d1c.oob.no_things", out.called_f0115_thing_pass, false,
                "ReDMCSB DUNGEON.C:1445-1475 F0151 wall fallback");
    return g_failures == before;
}

static int verify_source_evidence_mentions_all_anchors(void)
{
    int before = g_failures;
    uint8_t target[16 * 16];
    uint8_t viewport_pixels[DM1_VIEWPORT_WIDTH * DM1_VIEWPORT_HEIGHT];
    DM1_V1_D1CCenterFieldRenderPc34 out;
    const char *e;

    memset(target, 0, sizeof(target));
    memset(viewport_pixels, 0, sizeof(viewport_pixels));
    out = render_fixture(DM1_V1_D1C_CENTER_FIELD_PC34_ELEMENT_TELEPORTER,
                         false, false, false, false, false,
                         target, viewport_pixels);
    e = out.source_evidence;

    expect_nonnull("d1c.evidence.nonnull", e, "source evidence");
    expect_contains("d1c.evidence.contract", e, "Source-locked contract gate only",
                    "contract marker");
    expect_contains("d1c.evidence.f0151", e, "DUNGEON.C:1423-1479",
                    "ReDMCSB DUNGEON.C F0151");
    expect_contains("d1c.evidence.f0152", e, "DUNGEON.C:1481-1492",
                    "ReDMCSB DUNGEON.C F0152");
    expect_contains("d1c.evidence.f0153", e, "DUNGEON.C:1495-1506",
                    "ReDMCSB DUNGEON.C F0153");
    expect_contains("d1c.evidence.f0124", e, "F0124_DUNGEONVIEW_DrawSquareD1C",
                    "ReDMCSB DUNVIEW.C:7727-7958");
    expect_contains("d1c.evidence.f0128", e, "DUNVIEW.C:8530-8533",
                    "ReDMCSB DUNVIEW.C:8530-8533");
    expect_contains("d1c.evidence.f0098", e, "DUNVIEW.C:2962",
                    "ReDMCSB DUNVIEW.C F0098");
    expect_contains("d1c.evidence.f0113", e, "DUNVIEW.C:7942-7956",
                    "ReDMCSB DUNVIEW.C F0113");
    expect_contains("d1c.evidence.f0115", e, "DUNVIEW.C:7937",
                    "ReDMCSB DUNVIEW.C F0115");
    expect_contains("d1c.evidence.no_wall", e, "no F0100 wall bitmap",
                    "ReDMCSB DUNVIEW.C:7784-7872");
    expect_contains("d1c.evidence.d0c_reference", e,
                    "dm1_v1_viewport_d0c_center_field",
                    "ReDMCSB DUNVIEW.C:8164-8310");
    expect_bool("d1c.evidence.d0c_helper", out.called_d0c_reference_helper, true,
                "ReDMCSB DUNVIEW.C:8164-8310 d0c reference");
    return g_failures == before;
}

int main(void)
{
    int passed = 0;
    int total = 0;

    passed += verify_d1c_relative_square_path(); ++total;
    passed += verify_empty_center_field(); ++total;
    passed += verify_item_square(); ++total;
    passed += verify_creature_square(); ++total;
    passed += verify_projectile_and_explosion_square(); ++total;
    passed += verify_teleporter_square(); ++total;
    passed += verify_pit_square(); ++total;
    passed += verify_stairs_square(); ++total;
    passed += verify_open_door_square(); ++total;
    passed += verify_out_of_bounds_wall_fallback(); ++total;
    passed += verify_source_evidence_mentions_all_anchors(); ++total;

    if (g_failures) {
        printf("FAILURES: %d/%d assertions failed (%d/%d verify blocks passed)\n",
               g_failures, g_assertions, passed, total);
        return 1;
    }
    printf("PASS: %d assertions (%d/%d verify blocks)\n",
           g_assertions, passed, total);
    return 0;
}
